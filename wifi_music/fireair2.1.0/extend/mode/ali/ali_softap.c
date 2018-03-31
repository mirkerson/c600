
#ifdef __ALI_MUSIC__


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
#include <errno.h>

#include "ali_softap.h"
#include "ali_music.h"

#define SUCCESS 0
#define SSID_KEY		  "ssid"
#define TYPE_KEY		  "type"
#define PASSWORD_KEY	  "password"


static fd_set read_fds;
static int maxfd = 0;
static CLIENTX s_clientx[MAX_CLIENT];
static FILE *g_cfp = NULL;
static FILE *g_hfp = NULL;
static int g_time_counts = 0;



int ali_tcp_server_init()
{
	int i;
	int listenfd;
	int opt = SO_REUSEADDR;

	struct sockaddr_in server;
	
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Creating socket failed.");
		return -1;
	}
	
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(ALI_TCP_PORT);
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


int ali_tcp_server_loop(int listenfd)
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
	char buffer[20];

	fd_set tmp_fds = read_fds;
	tv.tv_sec = 5;  
    tv.tv_usec =0;  
	
	while (1)
	{
		 
		LOG_PRT("xxw waiting......");
		nready = select(maxfd + 1, &read_fds, NULL, NULL, &tv);
		if(nready < 0){
			
              LOG_PRT("Select() error is %d. ", nready);
		}else if(nready == 0){
              cnt++;
			  LOG_PRT("Select() timeout  %d.cnt=%d ", nready,cnt);
			  if(cnt == 30)return -1;
             
			  tv.tv_sec  = 3;  
              tv.tv_usec = 0;  
			  
		}else{

              LOG_PRT("Select() break and the return num is %d. ", nready);
		}

		
		if (FD_ISSET(listenfd, &read_fds))
		{
			LOG_PRT("Accept a connection.");
			sin_size = sizeof(struct sockaddr_in);
			if ((connectfd = accept(listenfd, (struct sockaddr *)&addr, (socklen_t *) & sin_size)) == -1)
			{
				perror("Accept() error");
				continue;
			}
			LOG_PRT("accept end");

			for (i = 0; i < MAX_CLIENT; i++)
			{
				if (s_clientx[i].fd < 0)
				{
					
					s_clientx[i].fd = connectfd; 
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer, "s_clientx[%.2d]", i);
					memcpy(s_clientx[i].name, buffer, strlen(buffer));
					s_clientx[i].addr = addr;
					memset(buffer, 0, sizeof(buffer));
					sprintf(buffer, "Only For Test!");
					memcpy(s_clientx[i].data, buffer, strlen(buffer));
					LOG_PRT("You got a connection from %s:%d.", inet_ntoa(s_clientx[i].addr.sin_addr),ntohs(s_clientx[i].addr.sin_port));
					LOG_PRT("Add a new connection:%s",s_clientx[i].name);
					break;
				}
			}
			
			if (i == MAX_CLIENT){
				LOG_PRT("Too many clients");
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
				LOG_PRT("Receive from connect fd[%d].", i);
				memset(recvbuf, 0, MAX_RECV_SIZE);
				
				if ((n = recv(sockfd, recvbuf, MAX_RECV_SIZE, 0)) == 0)//recv return 0 : client close
				{		   
					close(sockfd);	
					LOG_PRT("%s closed. User's data: %s", s_clientx[i].name, s_clientx[i].data);
					FD_CLR(sockfd, &tmp_fds);	 
					s_clientx[i].fd = -1;  
				}else{
					LOG_PRT("recv:%s\n",recvbuf);
					ret = ali_process_clientx(&s_clientx[i], recvbuf, n);
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

int ali_process_clientx(CLIENTX * client, char *recvbuf, int len)
{
	json_object *pRootObj = NULL,* pRepRootObj = NULL;
	const char *bssid= NULL ;
	const char *ssid= NULL,*password= NULL;
	char passworkvalue[128] = {0},ssidvalue[128] = {0};


	LOG_PRT("xxw recv:%s\n",recvbuf);
	
	pRootObj = json_tokener_parse(recvbuf);
	//pRootObj = json_tokener_parse("{\"header\": { \"msg_id\": \"365888\", \"action\": \"put\", \"dev_id\": \"FF0BDEFF\" },\"body\": { \"ssid\": \"TS1-xp\",\"password\": \"xzxy123456\",\"type\": \"wpa\" }}");
	if(pRootObj){

		bssid    = json_object_get_string(json_object_object_get(pRootObj, "bssid"));
	    password = json_object_get_string(json_object_object_get(pRootObj, "passwd"));
		ssid     = json_object_get_string(json_object_object_get(pRootObj, "ssid"));

		if(bssid&&password&&ssid){
    		LOG_PRT("ap_info:[%s:%s:%s]\n\n",ssid,password,bssid);
    		strcpy(passworkvalue,password);
    		strcpy(ssidvalue,ssid);
    		user_update_smt_file(ssidvalue,passworkvalue,NULL);	
		}else{


		}
			
	}else{
	    LOG_PRT("pRootObj NULL\n\n");
        return -1;
	
	}
	
    //create a respond to client
	pRepRootObj = json_object_new_object();
	
	char *pJsonString = NULL;

	if(pRepRootObj){
	
		//header
		json_object_object_add(pRepRootObj, "code", json_object_new_int(1000));
		json_object_object_add(pRepRootObj, "msg", json_object_new_string("format ok"));
		json_object_object_add(pRepRootObj, "model", json_object_new_string("BULL_LIVING_OUTLET_BULLSMART"));
		json_object_object_add(pRepRootObj, "mac", json_object_new_string("00:80:E1:BD:03:D5"));
		json_object_object_add(pRepRootObj, "sn", json_object_new_string("112233445566778899"));
		 
			
		pJsonString = strdup(json_object_get_string(pRepRootObj));
		FREE_JSON_OBJ(pRepRootObj);
		
	}else{
		FREE_JSON_OBJ(pRepRootObj);    
		return -1;
	}	
	LOG_PRT("ready to send back %s\n",pJsonString);
	
	send(client->fd, pJsonString, strlen(pJsonString), 0);
	FREE_MEM(pJsonString);
	FREE_JSON_OBJ(pRepRootObj);
	close(client->fd);
	
	return 0;
}


int ali_softap_mode_to_get_ap()
{
	
	int listenfd;
	int ret = 0;
		
	listenfd = ali_tcp_server_init();//create tcp server 
    if(listenfd < 0)
    {
		LOG_PRT("ali_tcp_server_init fail");
		return -1;
	}
	
    ret = ali_tcp_server_loop(listenfd);	
	
	close(listenfd); 
	
	return ret;
}

#endif

