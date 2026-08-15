#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <pthread.h>


#define SOCKET_PATH "/tmp/chat_uds" // 聊天通信地址
#define MAX_CLIENTS 10              // 最大支持连接数
#define BUFFER_SIZE 2048

// 全局变量：保存所有连接进来的客户端套接字
int client_fds[MAX_CLIENTS] = {0};
int client_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // 保护全局数组的互斥锁

// 专门处理跟一个客户端通信的线程函数
void* client_handler(void* arg) {
    int client_fd = *(int*)arg;
    free(arg); // 释放主线程传过来的动态内存
    char buffer[BUFFER_SIZE] = {0};

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        // 阻塞接收客户端发来的消息
        ssize_t ret = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (ret <= 0) {
            printf("客户端 fd=%d 断开连接。\n", client_fd);
            break;
        }

        printf("收到来自 fd=%d 的消息: %s\n", client_fd, buffer);

        // 判断是不是私聊指令（以 '@' 开头）
        if (buffer[0] == '@') {
            // 解析私聊指令：格式 "@目标fd 消息内容"
            int target_fd = -1;
            char msg[BUFFER_SIZE] = {0};
            // sscanf 从 buffer 中解析出 @后面的数字和剩余的消息
            if (sscanf(buffer, "@%d %[^\n]", &target_fd, msg) == 2) {
                pthread_mutex_lock(&lock);
                // 检查目标是否在当前的客户端列表中
                int found = 0;
                for (int i = 0; i < client_count; i++) {
                    if (client_fds[i] == target_fd) {
                        // 拼接私聊前缀，发给目标
                        char send_buf[BUFFER_SIZE*2] = {0};
                        snprintf(send_buf, BUFFER_SIZE*2, "[私聊] 来自 fd=%d: %s", client_fd, msg);
                        send(target_fd, send_buf, strlen(send_buf), 0);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    char err_msg[] = "私聊失败：目标未找到或已断开。\n";
                    send(client_fd, err_msg, strlen(err_msg), 0);
                }
                pthread_mutex_unlock(&lock);
            } else {
                send(client_fd, "私聊格式错误，请用：@目标fd 消息内容\n", 30, 0);
            }
        } else {
            // 不是私聊，就是群聊：把消息广播给【除自己外】的所有人
            pthread_mutex_lock(&lock);
            for (int i = 0; i < client_count; i++) {
                if (client_fds[i] != client_fd) {
                    char send_buf[BUFFER_SIZE*2] = {0};
                    snprintf(send_buf, BUFFER_SIZE*2, "[群聊] fd=%d: %s", client_fd, buffer);
                    send(client_fds[i], send_buf, strlen(send_buf), 0);
                }
            }
            pthread_mutex_unlock(&lock);
        }
    }

    // 处理断开连接：从全局数组中移除该 fd，并关闭连接
    pthread_mutex_lock(&lock);
    for (int i = 0; i < client_count; i++) {
        if (client_fds[i] == client_fd) {
            client_fds[i] = 0;
            // 将后面的元素往前移
            for (int j = i; j < client_count - 1; j++) {
                client_fds[j] = client_fds[j+1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&lock);

    close(client_fd);
    return NULL;
}

int main() {
    int server_fd = -1;
    int client_fd = -1;
    struct sockaddr_un server_addr = {0};//存储地址的专用结构体

    // 1. 创建 Unix 域流式套接字
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket失败"); exit(EXIT_FAILURE); }

    // 2. 清理旧socket文件并绑定
    unlink(SOCKET_PATH);
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind失败"); exit(EXIT_FAILURE);
    }

    // 3. 监听端口
    if (listen(server_fd, 5) == -1) { perror("listen失败"); exit(EXIT_FAILURE); }
    printf("【服务端启动】等待客户端连接... 输入 '@目标fd 内容' 可以模拟服务端发私聊(需单独实现)\n");

    // 4. 循环接收客户端连接
    while (1) {
        socklen_t client_len = sizeof(struct sockaddr_un);
        client_fd = accept(server_fd, NULL, &client_len);
        if (client_fd == -1) { perror("accept失败"); continue; }

        pthread_mutex_lock(&lock);
        if (client_count < MAX_CLIENTS) {
            // 将新客户端加入全局数组
            client_fds[client_count] = client_fd;
            client_count++;
            printf("新客户端 fd=%d 连入，当前在线人数: %d\n", client_fd, client_count);
            
            // 给该客户端发送专属 ID（用 fd 作为演示 ID）
            char welcome[BUFFER_SIZE] = {0};
            snprintf(welcome, BUFFER_SIZE, "=== 你的ID是: %d, 输入 @ID 消息 可私聊 ===\n", client_fd);
            send(client_fd, welcome, strlen(welcome), 0);
            pthread_mutex_unlock(&lock);

            // 5. 为每个客户端开辟一个独立线程去处理收发消息
            int* pfd = malloc(sizeof(int));
            *pfd = client_fd;
            pthread_t tid;
            pthread_create(&tid, NULL, client_handler, pfd);
            pthread_detach(tid); // 线程独立运行，不用 pthread_join 回收
        } else {
            pthread_mutex_unlock(&lock);
            close(client_fd);
            printf("连接数已达上限，拒绝连接 fd=%d\n", client_fd);
        }
    }
    return 0;
}