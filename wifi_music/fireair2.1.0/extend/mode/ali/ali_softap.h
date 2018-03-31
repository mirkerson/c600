#ifndef _ALI_SOFTAP_H_
#define _ALI_SOFTAP_H_

#include "common_def.h"
#include <stdbool.h>
#include <netinet/in.h>


#define ALI_TCP_PORT 65125    
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


#define ALI_SOFTAP_EXIT_TURE   0
#define ALI_SOFTAP_RUN  	   1




#endif 




