
#ifdef __SIMPLE_PLAY__

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
#include <json-c/json.h>
#include <assert.h>
#include <time.h>

#include "json.h"
#include "simple.h"


P_LIST_URL smp_list = NULL;

P_LIST_URL create_smp_list(int n, array_list* arr, char* str)
{
	int i = 0;
	P_LIST_URL head_pk = NULL;
	P_LIST_URL new_pk = NULL;
	P_LIST_URL tmp_pk = NULL;

    json_object* arr_obj;
    json_object *obj_url;
    
    int len;

    head_pk = (P_LIST_URL)malloc(sizeof(S_LIST_URL));
    if(head_pk == NULL)
        return NULL;

    head_pk->url = NULL;
    
    arr_obj = (json_object*)array_list_get_idx(arr,i); 

    obj_url = json_object_object_get(arr_obj, str);
    if(NULL != obj_url)
    {
        len = strlen(json_object_get_string(obj_url));
        if(len > 0)
        {
            head_pk->url = (char *)malloc(len + 1);    
            strcpy(head_pk->url, json_object_get_string(obj_url));
        }

    }
    
    head_pk->next = NULL;
    head_pk->prev = NULL;

	tmp_pk = head_pk;
    
	for(i=1; i<n; i++)
	{
        
		new_pk = (P_LIST_URL)malloc(sizeof(S_LIST_URL));
		if(new_pk == NULL)
			return NULL;

        new_pk->url = NULL;
        
        arr_obj = (json_object*)array_list_get_idx(arr,i); 

        obj_url = json_object_object_get(arr_obj, str);
        if(NULL != obj_url)
        {
            len = strlen(json_object_get_string(obj_url));
            if(len > 0)
            {
                new_pk->url = (char *)malloc(len + 1);    
                strcpy(new_pk->url, json_object_get_string(obj_url));
            }
        }

 		tmp_pk->next = new_pk;
        new_pk->prev = tmp_pk;

		new_pk->next = NULL;
        
        
		tmp_pk = new_pk;
        
	}

	tmp_pk->next = head_pk;
    head_pk->prev = tmp_pk;

    return head_pk;
    
}

void delete_smp_list(void)
{
	P_LIST_URL tmp_pk1;
	P_LIST_URL tmp_pk2;

    if(NULL == smp_list)
        return;

    tmp_pk1 = smp_list->next;
    tmp_pk2 = tmp_pk1->next;

	while(tmp_pk1 != smp_list)
	{
	    if(NULL != tmp_pk1->url)
	        free(tmp_pk1->url);
		free(tmp_pk1);
		tmp_pk1 = tmp_pk2;
		tmp_pk2 = tmp_pk2->next;
	}

    if(NULL != smp_list->url)
        free(smp_list->url);
	free(smp_list);
    
	smp_list = NULL;
}


static int timeout;

typedef struct _http_response
{
	long status;// http response code,.i.e.. 200, 302, 401
	double len;	// http response data length 
	char *data;	// http response data
}HTTP_RESPONSE;

static unsigned int process_response(char *buf, unsigned int size, unsigned int nmemb, void *stream)
{
	unsigned int last_len, len;
	HTTP_RESPONSE *response = (HTTP_RESPONSE *)stream;
	char *new_data = NULL;

	len = size * nmemb;
	last_len = response->len;

	timeout++;

	if(timeout >100)
	{
		timeout = 0;
		return OK;
	}

	new_data = malloc(len + last_len);
	if (NULL == new_data)
	{
		return ERROR;
	}

    if(NULL != response->data)
	    memcpy(new_data,response->data , last_len);
    
	memcpy(new_data + last_len, buf, len);
	response->data = new_data;

	response->len = last_len + len;

	return len;
}


int curl_parse(char *url)
{   
    CURL *curl = NULL;	
	CURLcode res;
	long code = 0;
	double len = 0;
	HTTP_RESPONSE *response = NULL;
    int ret;
    
	printf("curl_parse: %s\n", url);
    
	curl_global_init(CURL_GLOBAL_ALL); // init
	curl = curl_easy_init();	
	if(url == NULL || curl == NULL)
	{
		printf("param error");
    	curl_global_cleanup();
		return ERROR;
	}
		
	response = (HTTP_RESPONSE *)malloc(sizeof(HTTP_RESPONSE));
	
	if (response == NULL)
	{
		printf("malloc error");
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
	if (res != CURLE_OK)
	{
		printf("curl_easy_perform() res:%d failed: %s\n", res, curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    	curl_global_cleanup();
        del_http_res(response);
        response = NULL;
		return ERROR;
	}

	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);//获取响应码
	res |= curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &len);	
	response->status = code;
	response->len = len;

	printf("-------------------------------------------------\n");
	if(CURLE_OK == res && code == 200)
	{
		printf("ok code = %ld, len = %f\n", code, response->len);
        ret = http_res_parse(response->data); 
	}
	else
	{
		printf("code = %ld\n", code);
        ret = 1;
	}
	
	curl_easy_cleanup(curl);
	curl_global_cleanup();

    del_http_res(response);
    response = NULL;
	return ret;	
}

void del_http_res(HTTP_RESPONSE *response)
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


int http_res_parse(char *str)
{
    json_object *my_obj;

    int len;

    delete_smp_list();

    my_obj = json_tokener_parse(str);
    
    if(NULL == my_obj ) 
    {   
        printf("json_tokener_parse string failed!\n");
        return ERROR;
    }
    
    json_object *obj_data = my_obj;
    if(NULL == obj_data)
        return ERROR;


    int i=0;

    json_object_object_foreach(obj_data,key,val)
    {

        if (strcmp(key,"tracks") == 0)
        {
            if( (len = json_object_array_length(val)) > 0)
            {

                array_list* arr = json_object_get_array(val);
                if(NULL == arr)
                    return ERROR;

                printf("create_smp_list %d node!\n", len);
                
                smp_list = create_smp_list(len, arr, "play_url_64");//play_url_32
                
                for(i = 0; i < len; i++)
                {
                    printf("smp_list[%d]=%s\n", i, smp_list->url);
                    
                    if(NULL != smp_list->url)
                    {
                        //local_stop(0);
                        //local_play(smp_list->url, 0);
                        
                        json_object_put(my_obj);        
                        return OK;
                    }     
                    
                    smp_list = smp_list->next;
                }

                    delete_smp_list();
                        
                    json_object* arr_obj = (json_object*)array_list_get_idx(arr,0);     
                    if(NULL == arr_obj)
                        return ERROR;
                    
                    json_object_put(my_obj);
                    return ERROR;


            }
            else
            {
                json_object_put(my_obj);
                return ERROR;
            }
        }
    }
        
   // return OK;
}


#endif

