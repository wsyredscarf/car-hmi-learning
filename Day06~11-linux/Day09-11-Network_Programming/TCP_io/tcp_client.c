#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main(void) {
    int client_fd = -1;
    struct sockaddr_in server_addr = {0};
    char buffer[BUFFER_SIZE] = {0};

    // 1. 创建 TCP 套接字
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) { perror("socket失败"); exit(EXIT_FAILURE); }

    // 2. 配置要连接的服务器地址（本地 127.0.0.1 测试）
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);//htons把主机字节序转成网络字节序
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("地址转换失败"); exit(EXIT_FAILURE);
    }

    // 3. 发起连接
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("连接服务器失败"); close(client_fd); exit(EXIT_FAILURE);
    }
    printf("成功连接服务器！输入 'quit' 可退出。\n");

    // 4. 循环读键盘输入并发送
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // 去掉换行符

        if (strcmp(buffer, "quit") == 0) break;
        
        send(client_fd, buffer, strlen(buffer), 0);
        
        // 阻塞等待服务端把消息原样发回来
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        printf("服务器回射: %s\n", buffer);
    }

    close(client_fd);
    return 0;
}