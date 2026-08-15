#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/select.h>// select 模型核心头文件
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024
// select 最大限制是 1024，由系统宏 FD_SETSIZE 决定
#define MAX_FD 1024

int main(void) {
    // ====== 所有变量强制声明即初始化 ======
    int server_fd = -1;
    int conn_fd = -1;
    int max_fd = -1;
    struct sockaddr_in server_addr = {0};
    struct sockaddr_in client_addr = {0};
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};
    fd_set readfds = {0};   // 定义读描述符集合，声明即清零
    fd_set readfds_tmp = {0}; // 临时集合，用于每次循环重置

    // 1. 创建 TCP 套接字
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket 失败"); exit(EXIT_FAILURE); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind 失败"); close(server_fd); exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5) == -1) { perror("listen 失败"); close(server_fd); exit(EXIT_FAILURE); }

    // 2. 初始化 `readfds` 集合，把监听套接字 `server_fd` 放进去
    FD_ZERO(&readfds_tmp);
    FD_SET(server_fd, &readfds_tmp);
    max_fd = server_fd;

    printf("【Select 服务器启动】最大同时处理连接数: %d。\n", FD_SETSIZE);

    while (1) {
        // 3. 每次循环必须把当前监控的集合复制给一个临时变量，因为 select 会修改集合
        readfds = readfds_tmp; 

        // 4. 【核心函数】int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
        // 参数1：nfds 为所有文件描述符中最大值 + 1（本例最大是 server_fd）
        // 参数2：readfds 需要监测是否可读的集合
        // 参数5：timeout 设为 NULL 表示无限阻塞等待，直到有数据到达。
        int ready_num = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ready_num == -1) { perror("select 失败"); break; }

        // 5. 遍历所有文件描述符（性能退化点：每次都要从 0 遍历到 max_fd）
        for (int i = 0; i <= max_fd; i++) {
            // 如果当前 i 不在准备好的读集合里，跳过
            if (!FD_ISSET(i, &readfds)) continue;

            // 【新连接处理】
            if (i == server_fd) {
                conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (conn_fd == -1) { perror("accept 失败"); continue; }
                
                printf("新客户端连入, fd=%d\n", conn_fd);
                
                // 【关键点】把新的 conn_fd 加入集合，并更新 max_fd
                FD_SET(conn_fd, &readfds_tmp);
                if (conn_fd > max_fd) max_fd = conn_fd;
            } 
            // 【消息处理】
            else {
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t ret = recv(i, buffer, BUFFER_SIZE - 1, 0);
                if (ret <= 0) {
                    printf("客户端 fd=%d 断开连接。\n", i);
                    close(i);
                    FD_CLR(i, &readfds_tmp); // 【关键点】清理已断开的连接
                    if (max_fd == i) { // 如果断开的是当前最大的 fd，需要重新计算 max_fd
                        while (max_fd > 0 && !FD_ISSET(max_fd, &readfds_tmp)) max_fd--;
                    }
                } else {
                    printf("[fd=%d] 收到消息: %s\n", i, buffer);
                    send(i, buffer, strlen(buffer), 0);
                }
            }
        }
    }
    close(server_fd); 
    return 0;
}