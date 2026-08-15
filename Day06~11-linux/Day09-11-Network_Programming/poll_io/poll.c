#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h> // poll 核心头文件

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100 // poll 取消 1024 上限，改为可配置

int main(void) {
    // ====== 声明即初始化 ======
    int server_fd = -1;
    struct sockaddr_in server_addr = {0};
    char buffer[BUFFER_SIZE] = {0};
    int nfds = 0;
    
    // poll 使用结构体数组管理：fd(描述符), events(关心的事件), revents(发生的事件)
    struct pollfd fds[MAX_CLIENTS] = {0}; 
    fds[0].fd = -1; 
    fds[0].events = 0;

    // 1. 创建 socket 并绑定 (同 select，省略详细注释)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket 失败"); exit(EXIT_FAILURE); }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind 失败"); close(server_fd); exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5) == -1) { perror("listen 失败"); close(server_fd); exit(EXIT_FAILURE); }

    // 2. 【核心初始化】：将监听 socket 放入 pollfd 数组第一位
    fds[0].fd = server_fd;
    fds[0].events = POLLIN; // POLLIN 表示关心 可读 事件
    nfds = 1; // 当前有效 socket 的数量

    printf("【Poll 服务器启动】无 1024 上限，可配置为 %d。\n", MAX_CLIENTS);

    while (1) {
        // 3. 【核心函数】int poll(struct pollfd *fds, nfds_t nfds, int timeout);
        // 参数1：结构体数组指针；参数2：实际监控的 socket 数量；参数3：超时时间（-1 表示永久阻塞）
        int ready_num = poll(fds, nfds, -1);
        if (ready_num == -1) { perror("poll 失败"); break; }

        // 4. 遍历 pollfd 数组（性能退化点：依然要遍历 nfds 个元素）
        for (int i = 0; i < nfds; i++) {
            // 如果既没有可读数据，也没有异常，说明没事件发生，跳过
            if (!(fds[i].revents & POLLIN)) continue;

            // 【新连接处理】 如果是 server_fd 有事件，说明有新客户端
            if (fds[i].fd == server_fd) {
                struct sockaddr_in client_addr = {0};
                socklen_t client_len = sizeof(client_addr);
                int conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (conn_fd == -1) { perror("accept 失败"); continue; }

                printf("新客户端连入, fd=%d\n", conn_fd);

                // 将新客户端添加到 pollfd 数组中
                if (nfds < MAX_CLIENTS) {
                    fds[nfds].fd = conn_fd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                } else {
                    printf("客户端数量已达上限 %d,拒绝连接 %d\n", MAX_CLIENTS, conn_fd);
                    close(conn_fd);
                }
            } 
            // 【消息处理】
            else {
                memset(buffer, 0, BUFFER_SIZE);
                ssize_t ret = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
                if (ret <= 0) {
                    printf("客户端 fd=%d 断开连接。\n", fds[i].fd);
                    close(fds[i].fd);
                    // 【关键点】将数组中最后一个有效元素移到当前断开的位置，消除空洞
                    fds[i] = fds[nfds - 1]; 
                    nfds--;
                    i--; // 因为交换了，检查完当前 i 后，需要回退一步重新检查移过来的这个新 socket
                } else {
                    printf("[fd=%d] 收到消息: %s\n", fds[i].fd, buffer);
                    send(fds[i].fd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
    close(server_fd); return 0;
}