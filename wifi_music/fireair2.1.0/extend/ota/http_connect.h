/*
 * http_connect.h
 *
 *  Created on: 2016年1月13日
 *      Author: yangfuyang
 */

#ifndef LINUX_IMPL_INCLUDES_HTTP_CONNECT_H_
#define LINUX_IMPL_INCLUDES_HTTP_CONNECT_H_
#include <netdb.h>
#include <unistd.h>

#include "buffer.h"
#include "dbg.h"

#define RECV_SIZE 1024
#define BUF_SIZE  RECV_SIZE + 1

typedef struct download_info{
	char* url;
	char* dir;
}download_info;

int open_connection(char *hostname, char *port);
char* execute_request(char* host_name, char* port, char* request);
void asyn_download(char* string_url, char* dir);
void free_download_info(download_info* info);


#endif /* LINUX_IMPL_INCLUDES_HTTP_CONNECT_H_ */
