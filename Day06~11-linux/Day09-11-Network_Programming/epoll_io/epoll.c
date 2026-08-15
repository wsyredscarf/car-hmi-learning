#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> // 用于设置非阻塞 socket
#include <arpa/inet.h>
#include <sys/epoll.h> // epoll 核心头文件

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_EVENTS 100

// 【辅助函数】将 socket 设置为非阻塞模式
int set_nonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main() {
    // ====== 声明即初始化 ======
    int server_fd = -1;
    int conn_fd = -1;
    int epoll_fd = -1;
    struct sockaddr_in server_addr = {0};
    char buffer[BUFFER_SIZE] = {0};

    // 1. 创建 TCP 监听套接字并绑定、监听
    server_fd = socket(AF_INET, SOCK_STREAM, 0);


    // 参数1：sockfd = server_fd。配置创建出来的主监听套接字。
    // 参数2：level = SOL_SOCKET。表示要配置的是“套接字层”的通用选项，而不是 TCP 协议层。
    // 参数3：optname = SO_REUSEADDR。具体的选项名，意思是“允许重用本地地址和端口”。
    // 参数4：optval = &opt。把 &opt（值为 1）的地址传进去，告诉内核“1 代表开启这个功能”。
    // 参数5：optlen = sizeof(opt)。告诉内核传入的第四个参数占用了几个字节（这里是 int 4 个字节）
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//socket之后，bind之前使用

    server_addr.sin_family = AF_INET; 
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    // 2. 创建 epoll 
    epoll_fd = epoll_create1(0);
    
    // 3. 将监听套接字加入 epoll 关心的事件中
    struct epoll_event ev = {0};
    ev.events = EPOLLIN | EPOLLET; // 【核心区别】使用 ET 边缘触发模式！
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    // 4. 把 server_fd 设为非阻塞（ET模式下必须做的！）
    set_nonblocking(server_fd);

    struct epoll_event events[MAX_EVENTS] = {0};
    printf("【Epoll(ET模式) 服务器启动】真正的无上限高并发核心。\n");

    while (1) {
        // 5. 【核心函数】int epoll_wait(...);
        // 原理：内核通过红黑树保存 fd，通过回调机制通知。内核只返回发生了事件的 fd。
        int ready_num = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        
        for (int i = 0; i < ready_num; i++) {
            int current_fd = events[i].data.fd;
            
            if (current_fd == server_fd) {
                // ET 模式的 accept 处理：为了把所有等待连接处理干净，必须用 while 循环一直 accept！
                while ((conn_fd = accept(server_fd, NULL, NULL)) > 0) {
                    printf("新客户端连入, fd=%d\n", conn_fd);
                    set_nonblocking(conn_fd); // ET 模式客户端必须也是非阻塞
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
                }
            } else {
                // ET 模式读取数据：必须用 while 循环连续读，直到返回错误（EAGAIN/EWOULDBLOCK），否则会丢包！
                while (1) {
                    memset(buffer, 0, BUFFER_SIZE);
                    ssize_t ret = recv(current_fd, buffer, BUFFER_SIZE - 1, 0);
                    if (ret == 0) {
                        printf("客户端 fd=%d 断开。\n", current_fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                        close(current_fd);
                        break;
                    } else if (ret < 0) {
                        // 如果 errno 是 EAGAIN 或 EWOULDBLOCK，说明这次读完了，正常退出循环
                        break; 
                    } else {
                        printf("[fd=%d] 消息: %s\n", current_fd, buffer);
                        send(current_fd, buffer, strlen(buffer), 0);
                    }
                }
            }
        }
    }
    close(server_fd); close(epoll_fd); return 0;
}