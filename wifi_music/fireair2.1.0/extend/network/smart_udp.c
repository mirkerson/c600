#define LOG_TAG "smart_udp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <net/if.h> 
#include <net/if_arp.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h> 
#include "smart_ap_station.h"
#include "smart_udp.h"
#include <poll.h>
#include <json.h>


#if 1


pthread_t tid_server = 0;
static UDP_CLIENT g_clients[MAX_UDP_CLIENT];

int get_wlan0_ip(char *ip)
{
	int sock;	
	struct sockaddr_in sin;	
	struct ifreq ifr;	
	sock = socket(AF_INET, SOCK_DGRAM, 0);	
	if(sock == -1)	
	{		
		perror("socket");		
		return -1;	
	}	
	strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);	
	ifr.ifr_name[IFNAMSIZ - 1] = 0;	
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)	
	{		
		perror("iotcl");		
		return -1;	
	}	
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));	
	strcpy(ip, inet_ntoa(sin.sin_addr));
	close(sock);
	LOG_DEBUG("wlan0: %s", inet_ntoa(sin.sin_addr));
	return 0;
}


static void get_ip_port(int sockfd, char *ip, int *port)
{
	struct sockaddr_in out_addr;
	socklen_t serv_len = sizeof(out_addr);
	char serv_ip[32];
	
	getsockname(sockfd, (struct sockaddr *)&out_addr, &serv_len);
	inet_ntop(AF_INET, &out_addr.sin_addr, serv_ip, sizeof(serv_ip));
	*port = ntohs(out_addr.sin_port);
	get_wlan0_ip(ip);
	LOG_DEBUG("serv_ip:%s ip:%s:%d",serv_ip,ip, *port);
}

int  user_json_parse(char *buf)
{

    json_object *pRootObj,*pHeadObj,*pBodyObj;
	const char *action,*dev_id,*msg_id,*result_code;

	pRootObj = json_tokener_parse(buf);
	//pRootObj = json_tokener_parse("{\"header\": { \"msg_id\": \"365888\", \"action\": \"put\", \"dev_id\": \"FF0BDEFF\" },\"body\": { \"result_code\": \"0\",\"result_msg\": \"0\" }}");
	if(pRootObj)
	{
		pHeadObj = json_object_object_get(pRootObj, K_HEADER);
		pBodyObj = json_object_object_get(pRootObj, K_BODY);
		if(pHeadObj && pBodyObj)
		{
			action = json_object_get_string(json_object_object_get(pHeadObj, K_ACTION));
			dev_id = json_object_get_string(json_object_object_get(pHeadObj, "dev_id"));
			msg_id = json_object_get_string(json_object_object_get(pHeadObj, K_MSG_ID));

			result_code = json_object_get_string(json_object_object_get(pBodyObj, "result_code"));
			if(strcmp(result_code,"0")==0){
            	FREE_JSON_OBJ(pRootObj);
                return 0;
			}
		
		}
	}
	
	FREE_JSON_OBJ(pRootObj);
    return -1;
}

void *thread_recv_broadcast(void*arg)
{
	UDP_SERVER *pclient = (UDP_SERVER *)arg;
	char buf[1024];
	int addr_len, ret,ret_json;

	pthread_detach(pthread_self());
	addr_len = sizeof(pclient->srvaddr);
	LOG_DEBUG("thread_recv_audio running......");
	while (1)
	{
	    memset(buf,0,1024);
		ret = recvfrom(pclient->sockfd, buf, 1024, 0, 
			(struct sockaddr*)&pclient->srvaddr, (socklen_t *)&addr_len); 
		if (ret < 0){
			LOG_DEBUG("recv error %d\n", ret);
			break;
		}
		LOG_DEBUG("recv ip:%s  buff:%s\n",inet_ntoa(pclient->srvaddr.sin_addr),buf);
        
		ret_json = user_json_parse(buf);
		if(ret_json==0){
			LOG_DEBUG("ret_json = 0");
            break;            
		}		
		sleep(1);
	}
	pclient->is_work = false;
	LOG_DEBUG("thread_recv exit!!!");
	pthread_exit(0);
	return NULL;
}

void * thread_send_broadcast(void *arg)
{
	int i;
 	int ret;
	UDP_CLIENT *udp;
	UDP_SERVER *pclient = (UDP_SERVER *)arg;
	char * data = "xxw";
	struct sockaddr_in send_addr;
	json_object *pRepRootObj,*pRepHeadObj,*pRepBodyObj;
	
	int count = 0;

	pthread_detach(pthread_self());
	send_addr.sin_family=AF_INET;
    send_addr.sin_port=htons(PORT_SEND);
    send_addr.sin_addr.s_addr=inet_addr("255.255.255.255");
    bzero(&(send_addr.sin_zero),8);

    //create a respond to client
	pRepRootObj = json_object_new_object();
	pRepHeadObj = json_object_new_object();
	pRepBodyObj = json_object_new_object(); 

	char *p_json_string = NULL;

	if(pRepRootObj && pRepHeadObj && pRepBodyObj)
	{
		json_object_object_add(pRepRootObj, K_HEADER, pRepHeadObj);
		json_object_object_add(pRepRootObj, K_BODY, pRepBodyObj);

		//header
		json_object_object_add(pRepHeadObj, K_MSG_ID, json_object_new_string("365888"));
		json_object_object_add(pRepHeadObj, K_ACTION, json_object_new_string("put"));
		json_object_object_add(pRepHeadObj, "dev_id", json_object_new_string("FF0BDEFF"));

		//body
		json_object_object_add(pRepBodyObj, "wifi_connect_result", json_object_new_string("0"));
		json_object_object_add(pRepBodyObj, "result_msg", json_object_new_string("0")); 	
		p_json_string = strdup(json_object_get_string(pRepRootObj));
		
	}
	else
	{   
	    FREE_JSON_OBJ(pRepRootObj);
		FREE_JSON_OBJ(pRepHeadObj);    
		FREE_JSON_OBJ(pRepBodyObj);
		return NULL;
	}	
	LOG_DEBUG("ready to send %s len =%d\n",p_json_string,strlen(p_json_string));
	
	while(count < 40)
	{
	    for(i = 0; i < MAX_UDP_CLIENT; i++)
	    {
     		udp = &g_clients[i];
     		if(udp->alive)
     		{
     			ret = sendto(udp->session_server->sockfd, 
     							p_json_string, 
     							strlen(p_json_string), 
     							0, 
     							(struct sockaddr *)&send_addr, 
     							sizeof(struct sockaddr_in)); 
     				
     			if (ret < 0) 
     			{
     				LOG_DEBUG("sendto error: %s", strerror(errno));    					
     			}
				sleep(2);
				LOG_DEBUG("sendto...!!!");
     		}
     	}	
     	count++;
	}
	
	pclient->is_work = false;
	
	LOG_DEBUG("thread_send exit!!!");
	
	FREE_MEM(p_json_string);
	FREE_JSON_OBJ(pRepHeadObj);    
	FREE_JSON_OBJ(pRepBodyObj);
	FREE_JSON_OBJ(pRepRootObj);
	pthread_exit(0);
	return NULL;

}

static int udp_send_start(UDP_SERVER *s)
{
	if(s->tid_send > 0)
	{
		s->is_work = true;
		LOG_DEBUG("thread_send_already running...");
		return 0;
	}
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
	s->is_work = true;
	int ret = pthread_create(&s->tid_send, 
						&attr, 
						thread_send_broadcast, 
						s);
	if(ret < 0)
	{
		s->is_work = false;
		LOG_DEBUG("pthread_create error %s", strerror(errno));
		return -1;
	}
	pthread_attr_destroy(&attr);
	return 0;
}

static int udp_recv_start(UDP_SERVER *s)
{
	if(s->tid_recv> 0)
	{
		s->is_work = true;
		LOG_DEBUG("thread_recv_audio already running...");
		return 0;
	}
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
	s->is_work = true;
	int ret = pthread_create(&s->tid_recv, 
						&attr, 
						thread_recv_broadcast, 
						s);
	if(ret < 0)
	{
		s->is_work = false;
		LOG_DEBUG("pthread_create error %s", strerror(errno));
		return -1;
	}
	pthread_attr_destroy(&attr);
	return 0;
}


static int udp_send_stop(UDP_SERVER *s)
{
	s->is_work = false;
	return 0;
}

static int udp_recv_stop(UDP_SERVER *s)
{
	s->is_work = false;
	return 0;
}


UDP_SERVER *udp_server_create()
{
	UDP_SERVER *server = NULL;
	const int opt = 1;
	int nb = 0; 
	
	server = (UDP_SERVER *)malloc(sizeof(UDP_SERVER));
	if(server == NULL)
	{
		LOG_DEBUG("malloc error");
		return NULL;
	}
	memset(server, 0, sizeof(UDP_SERVER));
	server->sockfd = -1;
	server->is_work = false;
	server->send_start= udp_send_start;
	server->send_stop = udp_send_stop;
	server->recv_start= udp_recv_start;
	server->recv_stop = udp_recv_stop;
	
	server->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(server->sockfd < 0)
	{
		LOG_DEBUG("socket error %s", strerror(errno));
		FREE_MEM(server);
		return NULL;
	}
	
    //设置该套接字为广播类型，  
   
    nb = setsockopt(server->sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));  
    if(nb == -1)  
    {  
        LOG_DEBUG("set socket error...");  
        FREE_MEM(server);
        return false;  
    }  
  
	
	bzero(&server->srvaddr, sizeof(struct sockaddr_in));
	server->srvaddr.sin_family = AF_INET;
	server->srvaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	//server->srvaddr.sin_port = htons(0);//random port
    server->srvaddr.sin_port = htons(PORT_RECV);
	
	bind(server->sockfd, 
		(struct sockaddr *)&server->srvaddr, 
		sizeof(struct sockaddr_in));
	server->port = ntohs(server->srvaddr.sin_port);
	get_ip_port(server->sockfd, server->ip, &server->port);
	return server;
}

int udp_server_destory(UDP_SERVER *server)
{
	if(server->sockfd != -1)
	{
		close(server->sockfd);
		server->sockfd = -1;
	}
	
	if(server->tid_send != 0)
	{
		pthread_cancel(server->tid_send);
		pthread_join(server->tid_send, NULL);
		server->tid_send = 0;
	}
	
	if(server->tid_recv != 0)
	{
		pthread_cancel(server->tid_recv);
		pthread_join(server->tid_recv, NULL);
		server->tid_recv = 0;
	}
	return 0;
}

static void process(char *buf, unsigned int len, UDP_CLIENT *udp)
{
#if 0
	UDP_CMD cmd;
	//cmd = parser_cmd(buf);	
	LOG_DEBUG("------>UDP_CMD = %d", cmd);

	switch(cmd)
	{
		case UDP_CMD_CONNECT_WIFI_SUCCESS:
		{
			char ip[32] = {0};
			int port = 0;
			int fd;
			int ret;
			struct sockaddr_in addr;

			//ret = parser_ip_port(buf, &port, ip, sizeof(ip));
			LOG_DEBUG("remote video_server (%s:%d)", ip, port);
			
			fd = socket(AF_INET, SOCK_DGRAM, 0);
			bzero(&addr, sizeof(struct sockaddr_in)); 
			addr.sin_family = AF_INET; 
			inet_pton(AF_INET, ip, &addr.sin_addr); 
			addr.sin_port = htons(port); 

			udp->udp_server.sockfd = fd;
			udp->udp_server.servaddr = addr;			
			udp->session_server->recv_start(udp->session_server);
			
			break;
			
		}
			
		default:
		{
			LOG_DEBUG("not define command!!!");
			break;
		}
	}
	LOG_DEBUG("udp_client process over");
#endif
}

int udp_alive_num()
{
	int i;
	int n = 0;
	for(i = 0; i < MAX_UDP_CLIENT; i++)
	{   
	   if(g_clients[i].alive)
		  if(g_clients[i].session_server->is_work){
			  n++;
		  }
	}
	return n;
}


int udp_init()
{
	int i;
	UDP_CLIENT *udp;

	LOG_DEBUG("---------------->udp_init");
	for(i = 0; i < MAX_UDP_CLIENT; i++)
	{
		udp = &g_clients[i];
		udp->alive = false;
		udp->process = process;
		udp->udp_server.valid = false;
		udp->session_server = NULL;
	}
	return 0;
}

void udp_client_destory(UDP_CLIENT *udp)
{
	if(udp->session_server != NULL)
	{
		udp->session_server->send_stop(udp->session_server);
		udp->session_server->recv_stop(udp->session_server);
		udp_server_destory(udp->session_server);        
        FREE_MEM(udp->session_server);
		udp->session_server = NULL;
	}
	udp->alive = false;
}


void udp_exit()
{
	int i;
	LOG_DEBUG("---------------->udp_exit");
	for(i = 0; i < MAX_UDP_CLIENT; i++)
	{
		udp_client_destory(&g_clients[i]);
	}
}

UDP_CLIENT *udp_client_create()
{
	int i;
	for(i = 0; i < MAX_UDP_CLIENT; i++)
	{
		if(!g_clients[i].alive)
		{
			g_clients[i].alive = true;
			g_clients[i].session_server = udp_server_create();
			return &g_clients[i];
		}
	}
	return NULL;
}


#endif

int user_udp_server(void)
{  
   int i;
   int a_client_live = 0;
   
   udp_init();   
   UDP_CLIENT *udp = udp_client_create();
   udp->session_server->send_start(udp->session_server);  
   //udp->session_server->recv_start(udp->session_server); 
   while(1)
   {
   	   sleep(2);       
       //udp->process(NULL,0,udp);  
       LOG_DEBUG("udp live");
         
	   a_client_live = udp_alive_num();
	   if(a_client_live==0){
           break;
	   }
	     
   }
   udp_exit();
}

void *thread_udp_server(void *arg)
{
   int i;
   int a_client_live = 0;

   pthread_detach(pthread_self());
   udp_init();   
   UDP_CLIENT *udp = udp_client_create();
   udp->session_server->send_start(udp->session_server);  
   udp->session_server->recv_start(udp->session_server); 
   while(1){
   	   sleep(2);       
       //udp->process(NULL,0,udp);  
       LOG_DEBUG("udp live");      
	   a_client_live = udp_alive_num();
	   if(a_client_live==0){
           break;
	   }
	     
   }
   udp_exit();
   pthread_exit(0);
   return NULL;

}

int user_start_udp(void)
{
    int ret;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
    ret = pthread_create(&tid_server, 
						 &attr, 
						 thread_udp_server, 
						 0);
	if(ret < 0)
	{
		LOG_DEBUG("pthread_create error %s", strerror(errno));
		return -1;
	}
	pthread_attr_destroy(&attr);
	

}

int user_exit_udp(void)
{  
	LOG_DEBUG("tid_server=%ld",tid_server);
	sleep(2);
	if(tid_server){		
		udp_exit();
		sleep(1);
		pthread_cancel(tid_server);
		sleep(1);
		tid_server = 0;
	}
}

