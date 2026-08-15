#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>//TCP网络编程必须的头文件


#define PORT 8888  // 服务端绑定的端口
#define BUFFER_SIZE 1024

int main(void) {
    int server_fd = -1, client_fd = -1;//文件描述符
    struct sockaddr_in server_addr = {0};//TCP专用结构体
    struct sockaddr_in client_addr = {0};
    socklen_t client_len = sizeof(client_addr);//存储客户端大小
    char buffer[BUFFER_SIZE] = {0};

    // 1. 创建 TCP 套接字 (AF_INET 表示 IPV4，SOCK_STREAM 表示 TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket创建失败"); exit(EXIT_FAILURE); }

    // 2. 绑定 IP 和端口 (IP 填 INADDR_ANY 表示接受任何网卡发来的连接)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT); // htons把主机字节序转成网络字节序
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind失败"); close(server_fd); exit(EXIT_FAILURE);
    }

    // 3. 监听
    if (listen(server_fd, 5) == -1) { perror("listen失败"); close(server_fd); exit(EXIT_FAILURE); }
    printf("【TCP服务器】启动成功,正在等待客户端连接... (端口: %d)\n", PORT);

    // 4. 接受连接（先只处理一个客户端）
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) { perror("accept失败"); close(server_fd); exit(EXIT_FAILURE); }
    printf("客户端已连接 (IP: %s, 端口: %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 5. 循环收发数据
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t ret = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (ret <= 0) { printf("客户端断开连接。\n"); break; }

        printf("收到客户端消息: %s\n", buffer);
        
        // 原样把消息发回给客户端
        send(client_fd, buffer, strlen(buffer), 0); 
    }

    close(client_fd);
    close(server_fd);
    return 0;
}