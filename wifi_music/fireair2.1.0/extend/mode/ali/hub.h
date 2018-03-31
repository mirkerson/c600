//
// Created by dingzhu.ch on 15/6/17.
//
/**
 * HUB用于提供提供控制进程与播放器之间的进程通信
 * 定义如下两个概念:
 * 1. app: 与云端交互的应用程序,即播放控制逻辑部分
 * 2. host: 品牌方自由软件部分,主要是播放器和物理按键处理部分
 *
 * 在app与host之间的通信协议为json-rpc形式, 底层IPC机制为Datagram形式的Unix domain socket,
 * 以便简化报文边界处理问题. 考虑dgram的无连接特性,hub包含了两个socket描述符, 分别用于处理发送
 * 和接收报文.
 *
 * *注意*: 现阶段socket文件的路径位于 `/var/run/doug_demo`文件夹下，请注意运行者是否有读写权限
 */


#ifndef DOUG_OS_HUB_H
#define DOUG_OS_HUB_H
#include <sys/socket.h>
#include <sys/un.h>


#define HUB_APP  0   // 程序运行于app端
#define HUB_HOST 1  // 程序运行于host端

#define IPC_PATH "/tmp/"




/**
 * hub上下文对象，全局只需要一个实例即可
 */
struct hub_ctx {
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;
	int server_sockfd;
	int client_sockfd;
	char filename[104];
};

/**
 * 初始化hub上下文，负责完成套接字初始化, type参数表示当前运行环境是app端还是host端
 */
int hub_init(struct hub_ctx *ctx, int type);

/**
 * 销毁hub上下文，分别关闭两个套接字，并unlink监听的套接字文件
 */
void hub_destroy(struct hub_ctx *ctx);

/**
 * 阻塞接收数据，缓冲区buffer大小为len
 */
int hub_recv(struct hub_ctx *ctx, char *buffer, size_t len);

/**
 * 发送数据，缓冲区buffer大小为len
 */
int hub_send(struct hub_ctx *ctx, char *buffer, size_t len);


#endif //DOUG_OS_HUB_H
