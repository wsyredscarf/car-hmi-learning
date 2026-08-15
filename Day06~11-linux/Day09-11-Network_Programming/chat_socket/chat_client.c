#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/chat_uds"
#define BUFFER_SIZE 1024

int client_fd = -1;
// 全局标志位，控制接收线程和主循环运行
volatile sig_atomic_t g_running = 1; 

// 接收消息的线程函数
void* recv_thread(void* arg) {
    char buffer[BUFFER_SIZE] = {0};
    while (g_running) {
        memset(buffer, 0, BUFFER_SIZE);
        // 如果服务器端优雅退出了，或者这里出错了，recv 会返回 <= 0
        ssize_t ret = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (ret <= 0) {
            // 如果是因为 g_running 被设为 0 导致的停止，忽略错误
            if (g_running) {
                printf("\n服务器已断开连接。\n");
            }
            break;
        }
        // 打印收到的消息
        printf("\n%s\n", buffer);
        fflush(stdout); 
    }
    return NULL;
}

// 捕捉 Ctrl+C 信号，让客户端也能通过 Ctrl+C 安全退出
void handle_signal(int sig) {
    if (sig == SIGINT) {
        g_running = 0;
    }
}

int main() {
    struct sockaddr_un server_addr = {0};
    char input[BUFFER_SIZE] = {0};
    pthread_t tid;

    // 注册 Ctrl+C 处理函数
    signal(SIGINT, handle_signal);

    // 1. 创建套接字
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) { perror("socket失败"); exit(EXIT_FAILURE); }

    // 2. 连接服务端
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect失败,请先启动服务端！"); 
        exit(EXIT_FAILURE);
    }

    printf("=== 已加入群聊 ===\n");
    printf("-> 输入文字按回车，就是【群聊】\n");
    printf("-> 输入 '@连接号 文字' (例如: @6 你好)，就是【私聊】\n");
    printf("-> 输入 'quit' 或 'exit' 可退出聊天\n");

    // 3. 开启接收消息的独立线程
    pthread_create(&tid, NULL, recv_thread, NULL);

    // 4. 主线程负责循环读取用户键盘输入并发送
    while (g_running) {
        memset(input, 0, BUFFER_SIZE);
        
        // fgets 阻塞等待用户输入
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) break;
        
        // 去掉换行符
        input[strcspn(input, "\n")] = 0;

        // 【关键修改点】判断输入是否包含退出指令
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            g_running = 0; // 告诉接收线程该停了
            break;         // 跳出主循环
        }
        
        if (strlen(input) == 0) continue;

        // 发送给服务端
        send(client_fd, input, strlen(input), 0);
    }

    // ========== 核心清理动作 ==========
    // 1. 关闭 socket，让接收线程的 recv 立刻返回出错，从而自然退出
    close(client_fd); 
    
    // 2. 等待接收线程真正退出（防止内存泄漏和僵尸线程）
    pthread_join(tid, NULL); 

    printf("\n客户端已安全退出。\n");
    return 0;
}