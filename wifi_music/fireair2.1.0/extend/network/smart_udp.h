#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "smart_voice_common.h"

#define MAX_UDP_CLIENT 5
#define PORT_SEND 3309
#define PORT_RECV 3310


typedef enum _UDP_CMD
{
	UDP_CMD_NULL = 0,
	UDP_CMD_CONNECT_WIFI_SUCCESS,
}UDP_CMD;

//代表一个udp 服务对象
typedef struct _UDP_SERVER{
	int sockfd;    //当前服务的sock fd
	int port;      //当前服务的端口
	char ip[32];
	pthread_t tid_send;//当前 send 服务的线程id
	pthread_t tid_recv;//当前 recv 服务的线程id
	struct sockaddr_in srvaddr;//当前服务的地址
	struct sockaddr_in cltaddr;//收到的udp client地址
	int (*send_start) (struct _UDP_SERVER *s);//启动服务
	int (*send_stop) (struct _UDP_SERVER *s); //关闭服务
	int (*recv_start) (struct _UDP_SERVER *s);//启动服务
	int (*recv_stop) (struct _UDP_SERVER *s); //关闭服务
	int (*recv_audio_start) (struct _UDP_SERVER *s);//启动服务
	int (*recv_audio_stop) (struct _UDP_SERVER *s); //关闭服务
	bool is_work;
}UDP_SERVER;

typedef struct _REMOTE_UDP_SERVER
{
	bool valid;
	int sockfd;
	struct sockaddr_in servaddr; 
}REMOTE_UDP_SERVER;

typedef struct _UDP_CLIENT{//一个超级udp_client 
    bool alive; //一个事件对应一个client
    REMOTE_UDP_SERVER udp_server;
	UDP_SERVER *session_server;
	void (*process) (char *buf, unsigned int len, struct _UDP_CLIENT *udp);	
}UDP_CLIENT;


UDP_SERVER *udp_server_create();
int udp_server_destory(UDP_SERVER *s);
int user_udp_server(void);
int user_start_udp(void);
int user_exit_udp(void);



#endif

