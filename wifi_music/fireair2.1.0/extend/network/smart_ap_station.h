#ifndef _SMART_AP_STATION_H_
#define _SMART_AP_STATION_H_

//#include "plat_common.h"
#include <stdbool.h>
#include <netinet/in.h>


#define TCP_PORT 3308    
#define MAX_CLIENT 20
#define MAX_RECV_SIZE 1024     
#define MAX_SEND_SIZE 1024  
#define THREAD_STACK_SIZE (1024 * 1024)
#define BACKLOG 5   



#define K_HEADER		"header"
//#define K_MODULE		"module"
#define K_ACTION		"action"
#define K_SEND_TO		"send_to"
#define K_MSG_ID		"msg_id"
#define K_BODY_LEN		"body_len"

#define K_BODY			"body"


typedef struct _CLIENTX
{
    int fd;                     	
    char name[20];              	
    struct sockaddr_in addr;  
    char data[MAX_RECV_SIZE];     
} CLIENTX;


int user_run_softap(void);
int start_softap_mode_to_get_ap(void);
int process_clientx(CLIENTX * client, char *recvbuf, int len);


#endif 

