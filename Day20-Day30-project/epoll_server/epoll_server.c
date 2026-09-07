#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define PORT 8888
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

int main(int argc ,char* argv[])
{

    //1.=====创建套接字=====
    int server_fd = socket(AF_INET,SOCK_STREAM,0);
    if(server_fd == -1)
    {
        perror("创建socket失败!");
        exit(EXIT_FAILURE);
    } 
   
    int opt = 1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)); //设置端口复用，防止重启时端口被占用

    //2.=====绑定IP和端口=====
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; //监听所有网卡
    server_addr.sin_port = htons(PORT);
    if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1)
    {
        perror("绑定失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    //3.=====监听端口=====
    if(listen(server_fd,5) == -1)
    {
        perror("监听失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("[epoll服务器]正在监听端口 %d...\n",PORT);

    //4.=====使用epoll 多路IO复用=====
    int epoll_fd = epoll_create1(0);
    if(epoll_fd == -1){
        perror("epoll创建失败...");
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev ={0}; //把套接字加入epoll的监视列表（即告知有新的连接请叫我）
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_fd,&ev))//添加删除操作
    {
        perror("epoll_ctl失败");
        exit(EXIT_FAILURE);
    }
   struct epoll_event events[MAX_EVENTS]; // 存放就绪事件的数组

   //5.=====循环处理数据=====
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // 阻塞等待事件发生，-1 表示一直等
        if (nfds == -1) 
        {
            perror("epoll_wait失败");
            break; 
        }

        // 遍历所有就绪的事件
        for (int i = 0; i < nfds; i++) {
            int current_fd = events[i].data.fd;

            // 情况A：是监听套接字有事件 -> 说明有【新客户端】连进来了！
            if (current_fd == server_fd) {
                struct sockaddr_in client_addr = {0};
                socklen_t client_len = sizeof(client_addr);
                int conn_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                if (conn_fd == -1)
                { 
                    perror("accept失败");
                    continue;
                }
                printf("【新客户端】连入, fd=%d, IP=%s\n", conn_fd, inet_ntoa(client_addr.sin_addr));

                // 把新客户端 fd 加入 epoll 监视列表
                ev.events = EPOLLIN;
                ev.data.fd = conn_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);//添加fd到监视列表
            } 
            // 情况B：是普通的客户端套接字有事件 -> 说明【客户端发数据来了】或【断开了】！
            else {
                char buffer[BUFFER_SIZE] = {0};
                ssize_t ret = recv(current_fd, buffer, BUFFER_SIZE - 1, 0);

                if (ret <= 0) {
                    // ret == 0：客户端断开；ret < 0：出错
                    printf("【客户端】fd=%d 断开连接。\n", current_fd);
                    close(current_fd);                    // 关闭连接
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL); // 从监视列表移除
                } else {
                    printf("【收到消息】fd=%d: %s\n", current_fd, buffer);
                    // 回显：把收到的数据发回给客户端（模拟数据交互）
                    send(current_fd, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}