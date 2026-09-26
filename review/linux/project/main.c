/* ============================================================
 * main.c - 串口网关主程序
 * ============================================================
 * 架构：
 * - 主线程：epoll监听TCP + 串口
 * - 网络线程：从ring1读数据，发给TCP客户端
 * - 串口线程：从ring2读数据，写入UART
 * ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#include "ring_buffer.h"
#include "uart.h"
#include "modbus_parser.h"

#define MAX_EVENTS 64
#define TCP_PORT 8888

/* 全局变量 */
static int g_uart_fd = -1;
static int g_epfd = -1;
static RingBuffer g_ring_uart2net;  /* 串口→网络 */
static RingBuffer g_ring_net2uart;  /* 网络→串口 */
static int g_client_fd = -1;        /* 只支持单客户端 */
static ModbusParser g_modbus;

/* ============ 串口读线程 ============ */
void *uart_read_thread(void *arg) {
    char buf[1024];
    while (1) {
        /* read返回0表示无数据（VMIN=0），>0表示有数据 */
        ssize_t n = read(g_uart_fd, buf, sizeof(buf));
        if (n > 0) {
            /* 1. 原始数据存入ring1，等待发往网络 */
            rb_write(&g_ring_uart2net, buf, n);

            /* 2. 喂给Modbus状态机解析 */
            for (ssize_t i = 0; i < n; i++) {
                modbus_parse_byte(&g_modbus, (uint8_t)buf[i]);
            }
        } else if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(10000);  /* 10ms轮询间隔 */
            } else if (errno == EINTR) {
                continue;
            } else {
                perror("uart read");
                break;
            }
        }
    }
    return NULL;
}

/* ============ 网络读线程 ============ */
void *net_read_thread(void *arg) {
    char buf[1024];
    while (1) {
        if (g_client_fd < 0) {
            usleep(100000);  /* 无客户端，等待 */
            continue;
        }

        ssize_t n = read(g_client_fd, buf, sizeof(buf));
        if (n > 0) {
            /* 网络数据存入ring2，等待串口线程写回UART */
            rb_write(&g_ring_net2uart, buf, n);
        } else if (n == 0) {
            /* 客户端断开 */
            close(g_client_fd);
            g_client_fd = -1;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(10000);
            } else if (errno == EINTR) {
                continue;
            } else {
                perror("net read");
                break;
            }
        }
    }
    return NULL;
}

/* ============ 串口写线程 ============ */
void *uart_write_thread(void *arg) {
    char buf[1024];
    while (1) {
        int n = rb_read(&g_ring_net2uart, buf, sizeof(buf));
        if (n > 0) {
            write(g_uart_fd, buf, n);   /* 写入串口 */
        } else {
            usleep(10000);
        }
    }
    return NULL;
}



/* ============ 主函数 ============ */
int main() {
    /* 1. 初始化环形缓冲区 */
    rb_init(&g_ring_uart2net);
    rb_init(&g_ring_net2uart);

    /* 2. 打开串口（根据实际情况修改设备名和波特率） */
    g_uart_fd = uart_open("/dev/ttyS0", B115200);
    if (g_uart_fd < 0) return -1;
    set_nonblock(g_uart_fd);

    /* 3. 创建TCP监听socket */
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(lfd, 5);
    set_nonblock(lfd);

    /* 4. 创建epoll */
    g_epfd = epoll_create1(0);

    struct epoll_event ev, events[MAX_EVENTS];

    /* 把监听socket加入epoll */
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(g_epfd, EPOLL_CTL_ADD, lfd, &ev);

    /* 把串口fd加入epoll */
    ev.events = EPOLLIN | EPOLLET;   /* 串口用ET模式 */
    ev.data.fd = g_uart_fd;
    epoll_ctl(g_epfd, EPOLL_CTL_ADD, g_uart_fd, &ev);

    /* 5. 创建三个工作线程 */
    pthread_t tid1, tid2, tid3;
    pthread_create(&tid1, NULL, uart_read_thread, NULL);
    pthread_create(&tid2, NULL, net_read_thread, NULL);
    pthread_create(&tid3, NULL, uart_write_thread, NULL);

    /* 6. 主循环：epoll监听事件 */
    while (1) {
        int n = epoll_wait(g_epfd, events, MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == lfd) {
                /* 新客户端连接 */
                struct sockaddr_in cli;
                socklen_t cli_len = sizeof(cli);
                int cfd = accept(lfd, (struct sockaddr *)&cli, &cli_len);
                if (cfd < 0) continue;

                if (g_client_fd >= 0) {
                    close(g_client_fd);  /* 只支持单客户端，踢掉旧的 */
                }
                g_client_fd = cfd;
                set_nonblock(cfd);

                /* 加入epoll */
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = cfd;
                epoll_ctl(g_epfd, EPOLL_CTL_ADD, cfd, &ev);

                printf("client connected: %s:%d\n",
                       inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
            } else if (fd == g_uart_fd) {
                /* 串口数据由串口线程处理，这里只做ET模式读空 */
                char tmp[512];
                while (read(g_uart_fd, tmp, sizeof(tmp)) > 0);
            } else {
                /* 客户端数据由net_read_thread处理，这里只做ET模式读空 */
                char tmp[512];
                while (read(fd, tmp, sizeof(tmp)) > 0);
            }
        }
    }

    /* 7. 清理 */
    close(lfd);
    close(g_uart_fd);
    return 0;
}