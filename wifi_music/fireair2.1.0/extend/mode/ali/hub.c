//
// Created by dingzhu.ch on 15/6/17.
//

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "hub.h"

char *hub_app_filename  = IPC_PATH"hub_app.sock";
char *hub_host_filename = IPC_PATH"hub_host.sock";

int hub_init(struct hub_ctx *ctx, int type) 
{
	// start server side
	struct sockaddr_un *server_addr = &ctx->server_addr;
	memset(server_addr, 0, sizeof(struct sockaddr_un));
	server_addr->sun_family = AF_UNIX;
	strcpy(server_addr->sun_path,(type == HUB_APP) ? hub_app_filename : hub_host_filename);
	strcpy(ctx->filename, server_addr->sun_path);

	
	unlink(ctx->filename);
	ctx->server_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	int res = bind(ctx->server_sockfd, (struct sockaddr *)server_addr, sizeof(struct sockaddr_un));
	if (res) {
		perror("Fail to start hub server.\n");
		perror(strerror(errno));
		return res;
	}


	// initialize client side
	struct sockaddr_un *client_addr = &ctx->client_addr;
	client_addr->sun_family = AF_UNIX;
	strcpy(client_addr->sun_path,(type == HUB_APP) ? hub_host_filename : hub_app_filename);
	ctx->client_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (ctx->server_sockfd == -1 || ctx->client_sockfd == -1) {
		perror("Fail to create socket.");
		perror(strerror(errno));
		return -1;
	}
	return 0;

}

void hub_destroy(struct hub_ctx *ctx) {
	close(ctx->client_sockfd);
	close(ctx->server_sockfd);
	unlink(ctx->filename);
}


int hub_recv(struct hub_ctx *ctx, char *buffer, size_t len) {
	return recv(ctx->server_sockfd, buffer, len, 0);
}

int hub_send(struct hub_ctx *ctx, char *buffer, size_t len) {
	return sendto(ctx->client_sockfd, buffer, len, 0,
	              (struct sockaddr *)&ctx->client_addr,
	              sizeof(struct sockaddr_un));
}

