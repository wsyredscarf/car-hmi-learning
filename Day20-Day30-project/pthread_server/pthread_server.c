// ==================================================================
// 文件名：pthread_server_.c
// 功能：基于Linux C语言的多线程TCP群聊服务器（含信号退出、互斥锁保护）
// 编译命令：gcc pthread_server.c -o pthread_server -lpthread
// ==================================================================

// 1. 头文件包含区
#include <stdio.h>      // 标准输入输出：用于printf、perror等函数
#include <stdlib.h>     // 标准库：用于malloc、free、exit等函数
#include <string.h>     // 字符串处理：用于memset、strlen等函数
#include <unistd.h>     // Unix标准接口：用于read、write、close等函数
#include <signal.h>     // 信号处理：用于signal、SIGINT等
#include <pthread.h>    // 线程库：用于pthread_create、pthread_mutex_lock等
#include <sys/socket.h> // Socket接口：用于socket、bind、listen、accept等
#include <netinet/in.h> // 网络地址结构体：用于sockaddr_in、htons等
#include <arpa/inet.h>  // 网络地址转换：用于inet_ntoa等
#include <errno.h>      // 错误码判断：用于判断errno == EINTR（被信号打断）

// 2. 宏定义区：程序配置参数，方便后期统一修改
#define PORT 8888           // 服务器监听的端口号
#define MAX_CLIENTS 10      // 服务器允许同时连接的最大客户端数量

// 3. 全局变量区：程序中所有线程共享的数据
int client_fds[MAX_CLIENTS] = {0};                 // 数组：保存所有已连接客户端的Socket文件描述符（fd）
int client_count = 0;                              // 记录当前已连接的客户端数量
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;  // 互斥锁：防止多个线程同时修改全局数组导致数据错乱（防竞争）
volatile sig_atomic_t running = 1;                 // 运行标志：volatile防止编译器优化，sig_atomic_t保证信号安全

// 4. 函数声明区
// signal_handler：信号处理函数，当用户按下Ctrl+C (发送SIGINT信号) 时会被自动调用
void signal_handler(int sig) {
    // 判断收到的信号是否为SIGINT（中断信号，即Ctrl+C）
    if (sig == SIGINT) {
        // 打印退出提示，并使用fflush强制刷新输出缓冲区，确保立即显示
        printf("\n[系统消息] 收到退出信号，正在优雅关闭服务器...\n");
        fflush(stdout);
        // 将运行标志置为0，主循环检测到后会自动退出，并执行后续的清理工作
        running = 0;

 // 11. 服务器优雅关闭（清理资源）
    // 加锁，防止此时有线程在操作数组
    pthread_mutex_lock(&lock);
    // 遍历所有在线客户端，主动向它们发送关闭通知并关闭连接
    for (int i = 0; i < client_count; i++) {
        if (client_fds[i] > 0) {
            // 发送关闭通知给客户端，防止客户端傻傻等待“假死”
            send(client_fds[i], "服务器已关闭，连接断开。\n", strlen("服务器已关闭，连接断开。\n"), 0);

            // 先调用 shutdown 强制发送 FIN 包，告诉客户端立刻断开.(nc仅为测试网络互通的工具，所以和recv返回0有区别)
            shutdown(client_fds[i], SHUT_RDWR);
            // 主动关闭连接
            close(client_fds[i]);
        }
    }
        client_count = 0;
        pthread_mutex_unlock(&lock);
        exit(0);
    }
}

// client_handler：处理单个客户端通信的线程函数
// 参数：arg是一个void指针，在创建线程时传入（存放了客户端的fd）
void* client_handler(void* arg) {
    // 1. 类型转换与数据提取
    // 将void*强转为int*，并解引用获取客户端的文件描述符
    int fd = *(int*)arg;
    // 释放主线程分配的内存，防止内存泄漏
    free(arg);

    // 2. 定义接收缓冲区
    // buffer：用于存储从客户端接收到的数据，初始化为全0防止乱码
    char buffer[1024] = {0};

    // 3. 循环接收数据：只要服务器还在运行，就不断等待该客户端发送数据
    while (running) {
        // 清空缓冲区，确保每次读取都是干净的数据
        memset(buffer, 0, sizeof(buffer));
        // recv：阻塞等待接收客户端数据，直到有数据到达或连接关闭
        // 参数1：客户端fd；参数2：接收缓冲区；参数3：最大接收长度；参数4：标志位（0为默认）
        ssize_t ret = recv(fd, buffer, sizeof(buffer) - 1, 0);

        // 4. 判断接收返回值
        if (ret <= 0) {
            // ret == 0 表示客户端主动断开连接；ret < 0 表示接收出错（如网络异常）
            
            // 打印客户端断开消息（加锁打印，避免乱序）
            pthread_mutex_lock(&lock);
            printf("[系统消息] 客户端 fd=%d 断开连接。\n", fd);
            fflush(stdout);
            pthread_mutex_unlock(&lock);

            // 从全局客户端数组中移除已断开的客户端
            pthread_mutex_lock(&lock); // 加锁：修改共享数组前必须加锁
            for (int i = 0; i < client_count; i++) {
                if (client_fds[i] == fd) {
                    client_fds[i] = 0; // 找到该客户端的位置，将其置为0，表示已断开
                    client_count--;    // 客户端总数减1
                    break;             // 找到后直接退出循环
                }
            }
            pthread_mutex_unlock(&lock); // 解锁：修改完成必须解锁

            // 关闭该客户端的Socket描述符，释放资源
            close(fd);
            // 跳出循环，结束该线程
            break;
        } else {
            // 5. 打印日志：ret > 0 表示成功接收到数据
            // 加锁打印，防止多条线程同时printf导致输出乱序
            pthread_mutex_lock(&lock);
            printf("[收到消息] fd=%d: %s\n", fd, buffer);
            fflush(stdout); // 强制刷新，确保日志立即显示在屏幕上
            pthread_mutex_unlock(&lock);

            // 6. 实现“群聊”广播功能
            // 将接收到的消息，转发给除发送者之外的所有在线客户端
            pthread_mutex_lock(&lock); // 加锁：遍历共享数组前必须加锁
            for (int i = 0; i < client_count; i++) {
                // 如果当前遍历到的客户端不是发送者本人，且fd有效
                if (client_fds[i] != fd && client_fds[i] > 0) {
                    // 发送数据给其他客户端
                    send(client_fds[i], buffer, strlen(buffer), 0);
                }
            }
            pthread_mutex_unlock(&lock); // 解锁
        }
    }
    // 线程函数正常返回
    return NULL;
}

// 5. 主函数：程序的入口
int main(int vrgc, char* vrgv[]) {
    // 1. 注册信号处理函数
    // 告诉系统：当用户按下Ctrl+C（SIGINT）时，调用signal_handler函数
    signal(SIGINT, signal_handler);

    // 2. 创建Socket（监听套接字）
    // 参数1：AF_INET（IPv4）；参数2：SOCK_STREAM（TCP流式协议）；参数3：0（自动选择协议）
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("创建socket失败"); // perror会根据errno打印具体的错误原因
        exit(EXIT_FAILURE);       // 异常退出，返回非0值给操作系统
    }

    // 3. 设置Socket选项（端口复用）
    // SO_REUSEADDR：允许服务器重启时，立即重新使用处于TIME_WAIT状态的端口，避免端口占用报错
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 4. 绑定IP地址和端口,填充结构体
    struct sockaddr_in server_addr = {0}; // 定义服务器地址结构体并清零
    server_addr.sin_family = AF_INET;     // 设置协议族为IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY：监听本机的所有网卡IP地址
    server_addr.sin_port = htons(PORT);   // htons：将主机字节序转换为网络字节序（大端序）

    // bind：将Socket与IP和端口绑定
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind失败");
        close(server_fd); // 绑定失败必须关闭Socket，防止资源泄漏
        exit(EXIT_FAILURE);
    }

    // 5. 监听端口
    // 参数2：5表示允许排队等待的最大连接数
    if (listen(server_fd, 5) == -1) {
        perror("listen失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 打印服务器启动信息（强制刷新）
    printf("[系统消息] 服务器已启动，监听端口 %d 按下 Ctrl+C 优雅退出\n", PORT);
    fflush(stdout);//频繁的使用会影响性能，读取磁盘刷新数据，进行io操作

    // 6. 主循环：不断接受新连接和处理数据
    while (running) {
        // 7. 接受客户端连接
        // accept：阻塞等待客户端的连接请求
        int client_fd = accept(server_fd, NULL, NULL);

        // 8. 检查是否被信号打断（重要！解决你之前按Ctrl+C退不出去的问题）
        if (client_fd == -1) {
            // 如果被SIGINT信号打断，errno会变为EINTR，此时直接continue返回主循环检查running标志
            if (errno == EINTR) {
                continue;
            }
            // 其他情况：真正的错误，跳过本次循环
            continue;
        }

        // 9. 将新客户端加入全局数组
        pthread_mutex_lock(&lock); // 加锁，保护共享资源，防止竞争
        // 检查是否达到最大连接数限制
        if (client_count < MAX_CLIENTS) {
            client_fds[client_count++] = client_fd;
            printf("[系统消息] 新客户端连入 (fd=%d), 当前在线人数: %d\n", client_fd, client_count);
            fflush(stdout);
        } else {
            printf("[系统消息] 已达到最大连接数，拒绝连接 fd=%d\n", client_fd);
            close(client_fd); // 人数满时，直接关闭新连接
        }
        pthread_mutex_unlock(&lock); // 解锁

        // 10. 为该客户端创建独立线程
        // 【核心知识点】：为什么用malloc？
        // 如果直接传&client_fd，下次循环client_fd的值会被重新覆盖，导致线程里拿到的是错误的值
        int* pfd = malloc(sizeof(int));
        *pfd = client_fd;

        pthread_t tid;
        // 创建线程，执行client_handler函数，传入pfd
        pthread_create(&tid, NULL, client_handler, pfd);
        // 分离线程：告诉系统，当线程结束时自动回收它的资源，不需要主线程强制等待它
        pthread_detach(tid);
    }

    // 关闭监听套接字
    close(server_fd);
    printf("[系统消息] 服务器已安全退出，资源清理完成。\n");
    fflush(stdout);

    return 0;
}