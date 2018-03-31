
#include <stdio.h>  
#include <stdlib.h>  
#include <limits.h>  
#include <memory.h>  
#include <signal.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <netdb.h>  
#include <netinet/in.h>  
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>  

#include "rc.h"
#include "json.h"


#ifdef __REVERSE_CONTROL__


static pthread_mutex_t rc_lock;

static int connfd = -1;
static int listenfd;  //被动套接字(文件描述符），即只可以accept, 监听套接字

static int rc_enable = 0;

void rc_send_msg(char *pkg_buf, int pkg_size)
{
    int len;
    
    if (connfd == -1)
        return;
    
    pthread_mutex_lock(&rc_lock);

    len = send(connfd, pkg_buf, pkg_size, 0);
    if(len <= 0)
    {
        printf("send msg to client fail!\n");
    }


    pthread_mutex_unlock(&rc_lock);
    
}

int rc_init(void) 
{
    struct sockaddr_in servaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  //调用socket创建用于监听客户端的socket   
    {                           
        perror("Creating socket failed.");  
        return -1;  
    }  

    int opt = SO_REUSEADDR; 
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)  //设置socket属性
    {                           
        perror("setsockopt SO_REUSEADDR failed.");  
        return -1;  
    }
    
    bzero(&servaddr, sizeof(servaddr));  
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(RC_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 

    if (bind(listenfd, (struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
        return 1;

    if (listen(listenfd, SOMAXCONN) < 0) //listen应在socket和bind之后，而在accept之前
        return 1;
    
    struct sockaddr_in peeraddr; //传出参数
    socklen_t peerlen = sizeof(peeraddr); //传入传出参数，必须有初始值

    pthread_mutex_init(&rc_lock, NULL);

    rc_enable = 1;

    while (rc_enable)
    {
        connfd = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen);  //accept不再阻塞
        if (connfd == -1)
            return 1;

        printf("recv connect ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

        char recvbuf[1024] = {0};
        while (recv(connfd, recvbuf, 1024, 0))  
        {
            fputs(recvbuf, stdout);
        }

        close(connfd);
        connfd = -1;
        
    }

        
    return 0;
    
}


void rc_exit(void) 
{
    rc_enable = 0;

    pthread_mutex_destroy(&rc_lock);

    close(connfd);
    connfd = -1;

    close(listenfd);
    listenfd = -1;    

}

int rc_commit(int cmd)
{
    json_object *send_obj = json_object_new_object();    

    if(NULL == send_obj)
        return 1;
        
    char buf[128];
    int len = 0;
    printf("commit:%d\n", cmd);
    switch(cmd)
    {
        case RC_PLAY:
            json_object_object_add(send_obj, "cmd", json_object_new_string("play"));
            break;

        case RC_PAUSE:    
            json_object_object_add(send_obj, "cmd", json_object_new_string("pause"));
            break;

        case RC_STOP:    
            json_object_object_add(send_obj, "cmd", json_object_new_string("stop"));
            break;

        case RC_NEXT:    
            json_object_object_add(send_obj, "cmd", json_object_new_string("next"));
            break;

        case RC_PREVIOUS:    
            json_object_object_add(send_obj, "cmd", json_object_new_string("previous"));
            break;

        case RC_VOICE:    
            json_object_object_add(send_obj, "cmd", json_object_new_string("voice"));
            break;

        default:
            break;
    }

    json_object_object_add(send_obj, "type", json_object_new_int(RC_DEV_REQUEST)); 
      
    strcpy(buf, json_object_get_string(send_obj));
    len = strlen(buf);
    
    printf("buf=%s len=%d\n", buf, len);
    
    rc_send_msg(buf, len);
    
    json_object_put(send_obj);
        
    return 0;

}


void* rc_init_pthread(void *arg)
{
    pthread_detach(pthread_self());
    rc_init();
    pthread_exit(NULL);
}


#endif

