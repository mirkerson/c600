#define LOG_TAG "smart_ap_station"
#include "smart_ap_station.h"
#include "smart_voice_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <poll.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <poll.h>
#include <json.h>
//#include "smart_type.h"
#include <errno.h>
//#include "smart_udp.h"
//#include "plat_common.h"
#include <sys/time.h>  


#define SUCCESS 0
#define SSID_KEY		  "ssid"
#define TYPE_KEY		  "type"
#define PASSWORK_KEY	  "passwork"

#if 1//def LOG_TAG
#define LOG_DEBUG(fmt...)   	\
	    do {			\
	        printf("[D/%s] %s,line:%d: ", LOG_TAG, __FUNCTION__, __LINE__);\
	        printf(fmt);\
	        printf("\n");\
	    }while(0);	
#else
#define LOG_DEBUG(fmt, args...)
#endif

static fd_set read_fds;
static int maxfd = 0;
static CLIENTX s_clientx[MAX_CLIENT];
static FILE *g_cfp = NULL;
static FILE *g_hfp = NULL;
static int g_time_counts = 0;





//#define LOG_DEBUG printf 



int user_update_smt_file(char *ssid,char * password,char * type)
{
	
	char cmd[128]={0};
	if(ssid==NULL){
         return -1;
	}
	sprintf(cmd,"/usr/sbin/save.sh 1 WPA '%s' '%s'",ssid,password);
	system(cmd);
	system("/usr/sbin/save.sh 4");	
	return 0;
}

int user_file_exist(const char *name)
{
	if(name)
	{
		if(access(name, 0) == 0)
		{
			return 0;
		}
	}
	return -1;
}

int tcp_server_initx()
{
	int i;
	int listenfd;

	struct sockaddr_in server;
	
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Creating socket failed.");
		return -1;
	}
	
	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(TCP_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{  
		perror("Bind error.");
		return -1;
	}
	
	if (listen(listenfd, BACKLOG) == -1)
	{	
		perror("listen() error\n");
		return -1;
	}
	
	maxfd = listenfd;
	for (i = 0; i < MAX_CLIENT; i++)
	{
		s_clientx[i].fd = -1;
	}
	FD_ZERO(&read_fds); 		 
	FD_SET(listenfd, &read_fds);
	
	return listenfd;
}


int tcp_server_loopx(int listenfd)
{
	int nready;
	struct sockaddr_in addr;
	int sin_size; 
	int connectfd;
	int i;
	int maxi;
	int sockfd = -1;
	ssize_t n;
	char recvbuf[MAX_RECV_SIZE];
	int ret = -1;
	struct timeval tv; 
	int cnt = 0;

	fd_set tmp_fds = read_fds;
	tv.tv_sec = 5;  
    tv.tv_usec =0;  
	
	while (1)
	{
		 
		LOG_DEBUG("xxw waiting.....maxfd=%d,read_fds =%d",maxfd,read_fds);
		nready = select(maxfd + 1, &read_fds, NULL, NULL, &tv);	//调用select
		
		if(nready < 0){
			
              LOG_DEBUG("Select() error is %d. ", nready);
		}else if(nready == 0){
              cnt++;
			   LOG_DEBUG("Select() timeout  %d.cnt=%d ", nready,cnt);
			  if(cnt == 5)return -1;
             
			  tv.tv_sec = 30;  
              tv.tv_usec =0;  
		}else{

              LOG_DEBUG("Select() break and the return num is %d. ", nready);
		}

		
		if (FD_ISSET(listenfd, &read_fds))
		{
			LOG_DEBUG("Accept a connection.");
			sin_size = sizeof(struct sockaddr_in);
			if ((connectfd = accept(listenfd, (struct sockaddr *)&addr, (socklen_t *) & sin_size)) == -1)
			{
				perror("Accept() error");
				continue;
			}
			LOG_DEBUG("accept end");

			for (i = 0; i < MAX_CLIENT; i++)
			{
				if (s_clientx[i].fd < 0)
				{
					char buffer[20];
					s_clientx[i].fd = connectfd;   //保存客户端描述符
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer, "s_clientx[%.2d]", i);
					memcpy(s_clientx[i].name, buffer, strlen(buffer));
					s_clientx[i].addr = addr;
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer, "Only For Test!");
					memcpy(s_clientx[i].data, buffer, strlen(buffer));
					LOG_DEBUG("You got a connection from %s:%d.", inet_ntoa(s_clientx[i].addr.sin_addr),ntohs(s_clientx[i].addr.sin_port));
					LOG_DEBUG("Add a new connection:%s",s_clientx[i].name);
					break;
				}
			}
			
			if (i == MAX_CLIENT){
				LOG_DEBUG("Too many clients");
			}
			FD_SET(connectfd, &tmp_fds);
			if (connectfd > maxfd){
				maxfd = connectfd;	
			}
			if (i > maxi){
				maxi = i;
			}
			if (--nready <= 0){
				continue;		
			}
		}
		
		for (i = 0; i <= maxi; i++)
		{
			if ((sockfd = s_clientx[i].fd) < 0)
				continue;
			if (FD_ISSET(sockfd, &read_fds))
			{
				LOG_DEBUG("Receive from connect fd[%d].", i);
				memset(recvbuf, 0, MAX_RECV_SIZE);
				
				if ((n = recv(sockfd, recvbuf, MAX_RECV_SIZE, 0)) == 0)//recv return 0 : client close
				{		   
					close(sockfd);	
					LOG_DEBUG("%s closed. User's data: %s", s_clientx[i].name, s_clientx[i].data);
					FD_CLR(sockfd, &tmp_fds);	 
					s_clientx[i].fd = -1;  
				}else{
					LOG_DEBUG("recv:%s\n",recvbuf);
					ret = process_clientx(&s_clientx[i], recvbuf, n);
					//send(s_clientx[i].fd,"xxwlyh", 6, 0);
					//close(s_clientx[i].fd);

					LOG_DEBUG("ret = %d",ret);
					break;
				}
				if (--nready <= 0){
					break;		
				}
			}
		}

		read_fds = tmp_fds; 
		if(ret == 0)
			break;
	}

    return ret;
    
}


int process_clientx(CLIENTX * client, char *recvbuf, int len)
{
	json_object *pRootObj = NULL,*pHeadObj= NULL,*pBodyObj= NULL,*pRepRootObj= NULL,*pRepHeadObj= NULL,*pRepBodyObj= NULL;
	const char *action,*dev_id,*msg_id,*type= NULL;
	const char *ssid= NULL,*password= NULL;
	char typevalue[128] = {0},passworkvalue[128] = {0},ssidvalue[128] = {0};

	//local_play();
	LOG_DEBUG("xxw recv:%s\n",recvbuf);
	
	pRootObj = json_tokener_parse(recvbuf);
	//pRootObj = json_tokener_parse("{\"header\": { \"msg_id\": \"365888\", \"action\": \"put\", \"dev_id\": \"FF0BDEFF\" },\"body\": { \"ssid\": \"TS1-xp\",\"password\": \"xzxy123456\",\"type\": \"wpa\" }}");
	if(pRootObj)
	{
		pHeadObj = json_object_object_get(pRootObj, K_HEADER);
		pBodyObj = json_object_object_get(pRootObj, K_BODY);
		if(pHeadObj && pBodyObj)
		{
			action = json_object_get_string(json_object_object_get(pHeadObj, K_ACTION));
			dev_id = json_object_get_string(json_object_object_get(pHeadObj, "dev_id"));
			msg_id = json_object_get_string(json_object_object_get(pHeadObj, K_MSG_ID));

			type = json_object_get_string(json_object_object_get(pBodyObj, "type"));
			if(strcmp(type,"none") != 0)
				password = json_object_get_string(json_object_object_get(pBodyObj, "password"));
			ssid = json_object_get_string(json_object_object_get(pBodyObj, "ssid"));
			//server_ip = json_object_get_string(json_object_object_get(pBodyObj, "server"));
			LOG_DEBUG("ap_info:[%s:%s:%s]\n\n",ssid,password,type);
			strcpy(typevalue,type);
			if(strcmp(type,"none") != 0)
				strcpy(passworkvalue,password);
			strcpy(ssidvalue,ssid);
			
			user_update_smt_file(ssidvalue,passworkvalue,typevalue);	
			
		}else{
           return -1;
		}
	}else{
         return -1;
	}
	
    //create a respond to client
	pRepRootObj = json_object_new_object();
	pRepHeadObj = json_object_new_object();
	pRepBodyObj = json_object_new_object(); 

	char *pJsonString = NULL;

	if(pRepRootObj && pRepHeadObj && pRepBodyObj)
	{
		json_object_object_add(pRepRootObj, K_HEADER, pRepHeadObj);
		json_object_object_add(pRepRootObj, K_BODY, pRepBodyObj);		
	   
		//header
		json_object_object_add(pRepHeadObj, K_MSG_ID, json_object_new_string("12345"));
		json_object_object_add(pRepHeadObj, K_ACTION, json_object_new_string("put"));
		json_object_object_add(pRepHeadObj, "dev_id", json_object_new_string("FFFFFF"));
		 
		//body
		json_object_object_add(pRepBodyObj, "result_code", json_object_new_string("0"));
		json_object_object_add(pRepBodyObj, "result_msg", json_object_new_string("0")); 	
		pJsonString = strdup(json_object_get_string(pRepRootObj));
		FREE_JSON_OBJ(pRepRootObj);
	}
	else
	{
		FREE_JSON_OBJ(pRepHeadObj);    
		FREE_JSON_OBJ(pRepBodyObj);
		return -1;
	}	
	LOG_DEBUG("ready to send back %s\n",pJsonString);
	
	send(client->fd, pJsonString, strlen(pJsonString), 0);
	FREE_MEM(pJsonString);
	FREE_JSON_OBJ(pHeadObj);
	FREE_JSON_OBJ(pBodyObj);	
	FREE_JSON_OBJ(pRootObj);
	FREE_JSON_OBJ(pRepRootObj);
	close(client->fd);
	return 0;
}

int start_softap_mode_to_get_ap(void)
{
	int listenfd ;
	int ret;

	listenfd = tcp_server_initx();//create tcp server 
    ret = tcp_server_loopx(listenfd);	
	close(listenfd); 
	
	return ret;
}

int user_run_softap(void)
{
	int ret;

    LOG_DEBUG("--xxw--->connect ap start");
    //start_time = get_time_ms();
	ret = start_softap_mode_to_get_ap();
	//end_time = get_time_ms();
	//LOG_DEBUG("-------->connect ap end %u ms", start_time-end_time);

	return ret;
}

