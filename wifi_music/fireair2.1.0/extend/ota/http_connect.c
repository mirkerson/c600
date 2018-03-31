/*
 * http_connect.c
 *
 *  Created on: 2016年1月13日
 *      Author: yangfuyang
 */
#include <stdio.h>
#include <pthread.h>
#include<sys/stat.h>
#include "http_connect.h"

#include "utils.h"
#include "dbg.h"

void download(char* string_url, char* dir){
	Url* url = url_parse(string_url);
	printf(" host %s path %s ", url->hostname, url->path);
	int socketfd;
	char http_protol[RECV_SIZE];
	int ret;
	float http_ver = 0.0;
	int status;
	int write_length;
	int file_len;
	char recvbuf[RECV_SIZE]; /* recieves http server http protol HEAD */
	char buffer[RECV_SIZE];
	FILE *fp = NULL;
	void *start = NULL;
	bzero (http_protol, sizeof (http_protol));
	bzero (recvbuf, sizeof (recvbuf));
	socketfd = open_connection(url->hostname, url->port);
	if(socketfd < 0){
		debug("open socket failed ");
		return;
	}
	sprintf (http_protol, "GET %s HTTP/1.1\r\n" \
			"Host: %s\r\n" \
	        "Conection: Keep-Alive\r\n\r\n",
	        url->path, url->hostname);
    ret = write (socketfd, http_protol, strlen (http_protol));
    if (ret == -1)
    {
    	debug ("write failed:%d\n", errno);
        exit (1);
    }

    ret = read (socketfd, recvbuf, sizeof (recvbuf));
    if (ret == 0)
    {
    	debug ("server closed:%d\n", errno);
        exit (1);
    }
    else if (ret == -1)
    {
    	debug ("read failed:%d\n", errno);
        exit (1);
    }

    debug ("%s", recvbuf);
    sscanf (strstr (recvbuf, "HTTP/"), "HTTP/%f %d", &http_ver, &status);
    sscanf (strstr (recvbuf, "Content-Length"), "Content-Length: %d", &file_len);

    if (status != 200 || file_len == 0)
    {
    	debug ("http connect failed!\n");
        exit (1);
    }
   if(mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)){
	   debug(" make dir %s  failed", dir);
   }
   	char* name_from_path = get_name_from_url(url->path);
   	debug("name from path string : %s ", name_from_path);
    Buffer* filename = buffer_alloc(BUF_SIZE);
    buffer_appendf(filename, dir);
    buffer_appendf(filename, name_from_path);
    char* file_name = buffer_to_s(filename);
    fp = fopen(file_name, "wb");
    buffer_free(filename);
    free(name_from_path);
    if (fp == NULL)
    {
    	debug ("File:%s Can not open:%d\n", file_name, errno);
        exit (1);
    }

    bzero (buffer, sizeof (buffer));

    /* download file's address start here whithout http protol HEAD */
    start = (void *) strstr(recvbuf, "\r\n\r\n") + sizeof ("\r\n\r\n")-1;
    fwrite (start, sizeof (char), ret - ((void *)start - (void *)&recvbuf), fp);

    while (1)
    {
        ret = read (socketfd, buffer, sizeof (buffer));

        if (ret == 0) break; /* download finish */

        if (ret < 0)
        {
        	debug ("Recieve data from server [%s] failed!\n", url->hostname);
            break;
        }

        write_length = fwrite (buffer, sizeof (char), ret, fp);
        if (write_length < ret)
        {
        	debug ("File: %s write failed.\n", file_name);
            break;
        }
        bzero (buffer, sizeof (buffer));
    }

    debug ("\ndownload %s file finish.\n", file_name);

    close (socketfd);
    fclose (fp);
    free(file_name);
    url_free(url);
}

void free_download_info(download_info* info){
	if(NULL != info){
		free(info->url);
		free(info->dir);
		free(info);
	}
}

void* download_runner(void* args){
	download_info* info = (download_info*)args;
	debug(" url string : %s dir %s ", info->url, info->dir);
	download(info->url, info->dir);
	free_download_info(info);
	pthread_exit(0);
}

void asyn_download(char* string_url, char* dir){
	debug(" asyn download url : %s   dir %s ", string_url, dir);
	download_info* info = (download_info*)malloc(sizeof(download_info));
	info->url = copy_str(string_url);
	info->dir = copy_str(dir);
	pthread_t tid;
	//pthread_attr_t attr;
	//pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(&tid,NULL,download_runner, info);
    pthread_join(tid, NULL);
	//pthread_attr_destroy(&attr);
}

char* execute_request(char* host_name, char* port, char* request){
	int socketfd = 0;
	int pcontent_size = 0;
	int send_byte, totalsend, nbytes;

	char* content = NULL;
	int i = 0, b=0;
	char buffer[1024];

	int content_size, index = 0;
	socketfd = open_connection(host_name, port);
	if(socketfd <=0){
		printf(" execute request open socket error ");
		return NULL;
	}
	/*发送http请求request*/
	send_byte = 0;
	totalsend = 0;
	nbytes=strlen(request);
	while(totalsend < nbytes)
	{
		send_byte = send(socketfd, request + totalsend, nbytes - totalsend, 0);
		if(send_byte==-1)
		{
			printf("send error!%s\n", strerror(errno));
			return NULL;
		}
		totalsend+=send_byte;
		printf("%d bytes send OK!\n\n", totalsend);
	}

	/* 连接成功了，接收http响应，response */
	i = 0;
	b = 0;
	while(i < 4 && recv(socketfd,&buffer[b],1, 0)==1 )
	{
		if(buffer[b] == '\r' || buffer[b] == '\n')
			i++;
		else
			i = 0;
		//printf("%c", buffer[b]);/*把http头信息打印在屏幕上*/
		b++;
	}
	buffer[b] = '\0';
	char* temp_s = strstr(buffer, "Content-Length: ");
    printf("--------buffer:%s\n", buffer);

	if( temp_s != NULL )
	{
		sscanf(temp_s, "Content-Length: %d", &pcontent_size);
		content = malloc(pcontent_size + 1);
        if(NULL == content)
            return NULL;
        
		content_size = pcontent_size;
		while(content_size > 0)
		{
			nbytes = recv(socketfd, &content[index], pcontent_size-index, 0);
			if( nbytes == 0 || nbytes == -1 )
			{
				printf("recv end!\n");
				break;
			}
			content_size -= nbytes;
			index += nbytes;
		}
        
		if( content_size != 0 )
			printf("recv content size (%d != %d)\n", index ,pcontent_size);
		content[pcontent_size] = '\0';
	}
	else
	{
		//printf("buffer:%s\n", buffer);
	}
      
	if(socketfd) close(socketfd);
	return content;
}


int open_connection(char *host, char *port){
	int sockfd, status;
	struct addrinfo *res = NULL;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(host, port, &hints, &res);
    if(status != 0){
    	debug("Could not resolve host: %s\n", gai_strerror(status));
    	return -1;
    }
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    jump_unless(sockfd > 0);

    status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    jump_unless(status == 0);
    return sockfd;
    error:
        return -1;
}


