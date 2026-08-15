#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>


// 这个路径必须和服务器代码里的 SOCKET_PATH 完全一模一样
#define SOCKET_PATH "/tmp/uds_socket"
#define BUFFER_SIZE 1024

int main(void) {
    
    int client_fd; // 客户端的单套接字描述符
    struct sockaddr_un server_addr; // 存储要连接的服务端地址
    char buffer[BUFFER_SIZE] = {0};

    // ========== 1. 创建客户端套接字 ==========
    // 客户端同样必须创建 AF_UNIX 和 SOCK_STREAM
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("客户端 socket 创建失败");
        exit(EXIT_FAILURE);
    }

    // ========== 2. 指定要连接的服务端地址 ==========
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    // 将目标文件路径拷贝到 sun_path 中，告诉内核我要连哪个路径
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // ========== 3. 向服务端发起连接请求 ==========
    // connect 会尝试去连接服务端。
    // 如果服务端没启动，或者路径不对，connect 会立刻返回 -1。
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("客户端 connect 连接失败（请确保服务端已先启动！）");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf("【UDS 客户端】成功连接到服务端！\n");

    // ========== 4. 发送请求并读取回复 ==========
    const char *msg = "你好，我是 HMI 前端，请把当前车速发给我！";
    
    // 将请求发送给服务端（+1 包含结束符 \0）
    if (write(client_fd, msg, strlen(msg) + 1) == -1) {
        perror("客户端发送数据失败");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    printf("客户端发送: %s\n", msg);

    // 清空接收缓冲区
    memset(buffer, 0, BUFFER_SIZE);
    // 阻塞等待服务端回复的消息
    if (read(client_fd, buffer, BUFFER_SIZE - 1) == -1) {
        perror("客户端读取服务端回复失败");
    } else {
        printf("客户端收到服务端回复: %s\n", buffer);
    }

    // ========== 5. 清理并结束 ==========
    close(client_fd);
    printf("【UDS 客户端】正常退出。\n");
    return 0;
}