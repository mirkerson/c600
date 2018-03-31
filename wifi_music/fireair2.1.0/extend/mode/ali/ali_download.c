
#ifdef __ALI_MUSIC__

#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "openssl/ssl.h"
#include "openssl/err.h"
#include <netdb.h>
#include <curl/curl.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>

#include "common_def.h"

static FILE *dl_fp = NULL;
static int dl_over = 1;
static char dl_url[1024];

typedef struct _http_response
{
	long status;// http response code,.i.e.. 200, 302, 401
	double len;	// http response data length 
	char *data;	// http response data
}HTTP_RESPONSE;

static unsigned int process_response(char *buf, unsigned int size, unsigned int nmemb, void *stream)
{
	uint last_len, len;
    HTTP_RESPONSE *response = (HTTP_RESPONSE *)stream;

	len = size * nmemb;
	last_len = response->len;

	fwrite(buf,len,1,dl_fp);

	response->len = last_len + len;
    
	return len;

}

static void del_http_res(HTTP_RESPONSE *response)
{
    if(NULL != response)
    {
        if(NULL != response->data)
        {
            free(response->data);
            response->data = NULL;
        }
            
        free(response);
    }        
}

int curl_download(char *url)
{   
    CURL *curl = NULL;	
	CURLcode res;
	long   code = 0;
	double len = 0;
	HTTP_RESPONSE *response = NULL;
    int ret;

    LOG_PRT("curl_parse: %s", url);

	curl_global_init(CURL_GLOBAL_ALL); // init
	
	curl = curl_easy_init();	
	if(url == NULL || curl == NULL)
	{
		LOG_PRT("param error");
        curl_global_cleanup();
		return ERROR;
	}

	if(curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60) != CURLE_OK)
	{
		LOG_PRT("set post file time out err!\n");
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return ERROR;
	}
		
	response = (HTTP_RESPONSE *)malloc(sizeof(HTTP_RESPONSE));
	if (response == NULL){
		LOG_PRT("malloc error");
        curl_easy_cleanup(curl);
    	curl_global_cleanup();          
		return ERROR;
	}
	memset(response, 0, sizeof(HTTP_RESPONSE));

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &process_response);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	res = curl_easy_perform(curl);
	if (res != CURLE_OK){

		LOG_PRT("curl_easy_perform() res:%d failed: %s", res, curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	    curl_global_cleanup();
        del_http_res(response);
        response = NULL;
		return ERROR;

	}

	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);//»ñÈ¡ÏìÓ¦Âë
	res |= curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &len);	
	response->status = code;
	response->len = len;

	if(CURLE_OK == res && code == 200){
		LOG_PRT("ok code = %ld, len = %f", code, response->len);
        ret = 0;
	}else{
		LOG_PRT("code = %ld", code);
        ret = 1;
	}
	
	curl_easy_cleanup(curl);
	curl_global_cleanup();

    del_http_res(response);
    response = NULL;
    
	return ret;	
    
}

static int get_max_file_num(const char * path)
{
    DIR *dirp = NULL;
    int max_file_num = 0;
    int ret;
    
    if ( (ret = access(path, F_OK)) < 0 )
        return -1;
        
	if(NULL== (dirp=opendir(path)))	
		return 0;

    while ( NULL != readdir(dirp) )
    {
        max_file_num++;
    }

    return max_file_num;
}

void* curl_download_thread(void *arg)
{  
	pthread_detach(pthread_self());

    char file_path[128];
    int f_cnt;
    char *url = (char *)arg;
    char del_cmd[132];
    int ret;

    LOG_PRT("downloading....%s", url);
    
    f_cnt = get_max_file_num("/mnt/sdcard/CUCKOO");

    if(f_cnt >= 0 && f_cnt <= 100)
    {
        do
        {
            memset(file_path, 0, 128);
            sprintf(file_path, "/mnt/sdcard/CUCKOO/CA%d.mu", f_cnt++);  
        }              
        while( 0 == access(file_path, 0) );
    
        LOG_PRT("f_cnt = %d", --f_cnt);        
    
        if ((dl_fp = fopen(file_path, "w")) == NULL)     
        {
            perror("Open file failed\n");
        }
        else
        {
            int cnt = 0;

            while( 0 != (ret = curl_download(url)) )
            {
                if(cnt++ > 3)
                {
                    sprintf(del_cmd,"rm %s",file_path);
                    system(del_cmd);

                    break;
                }
                    
            }
            
            //ret = curl_download(url);
            
            fclose(dl_fp);
            
        }
                  
    }

    dl_over = 1;     

	pthread_exit(NULL);

}

int download_file(char* url)
{
	int ret;
  	pthread_t tid;
    
	if(url == NULL)
    { 
		LOG_PRT("url NULL");	
        return -1;
	}
    
    if(!dl_over)
        return -1;
    
    dl_over = 0;
    strcpy(dl_url, url);
	ret = pthread_create(&tid, NULL, &curl_download_thread, dl_url);  
	if(ret != 0) {  
		LOG_PRT("pthread_create curl_download_thread failed");	
        return -1;
	} 

    return ret;

}

#endif

