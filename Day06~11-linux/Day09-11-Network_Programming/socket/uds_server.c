#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>


// 定义通信的本地文件路径。双方必须绝对一致
// "/tmp" 目录是 Linux 的临时目录，所有用户都有权限创建和写入
#define SOCKET_PATH "/tmp/uds_socket" // 套接字文件路径（通信地址）

// 定义读写缓冲区的最大长度
#define BUFFER_SIZE 1024

int main (void)
{
    //创建套接字文件描述符
    int server_fd,client_fd;
    
    // sockaddr_un 是 Unix 域专用的地址结构体
    // server_addr：存储服务端绑定的地址；client_addr：存储连接进来的客户端地址
    struct sockaddr_un server_addr,client_addr;

    // client_len：用来存放客户端地址结构体的大小，传入 accept 函数
    socklen_t client_len;

    // 存放读写数据的缓存数组，初始化为全 0 防止乱码
    char buffer[BUFFER_SIZE] = {0};
    // num_read：记录 read 函数实际读到的字节数
    ssize_t num_read;

    // ========== 1. 创建 Unix 域套接字 ==========
    // 参数1：AF_UNIX（或 AF_LOCAL）表示使用本地文件系统进行进程间通信（不用走网络协议栈，效率极高）
    // 参数2：SOCK_STREAM 表示流式套接字（类似 TCP，面向连接，保证顺序和可靠传输）
    // 参数3：0 表示使用默认协议（对于 AF_UNIX 和 SOCK_STREAM，默认是 0）
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("服务端 socket 创建失败"); // perror 会根据全局 errno 打印具体的错误描述
        exit(EXIT_FAILURE); // 非 0 状态码退出，通知操作系统程序异常结束
    }

    // ========== 2. 清理残留的套接字文件 ==========
    // 如果上一次程序崩溃了，/tmp/uds_socket 文件可能还会残留在磁盘。
    // 如果不删除，下一次 bind() 绑定会直接失败。unlink 用于删除这个文件。
    unlink(SOCKET_PATH);

    // ========== 3. 初始化并绑定地址 ==========
    // 使用 memset 把结构体全部清零，防止里面有脏数据导致绑定失败
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    // 指定协议族为 AF_UNIX
    server_addr.sun_family = AF_UNIX;
    // 将文件路径拷贝到 sun_path 成员中。
    // 减 1 是为了确保如果路径过长，也不会导致数组越界覆盖。
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // bind 函数用于把刚才创建的 socket 和特定的文件路径绑定在一起。
    // 参数1：套接字描述符；参数2：地址结构体指针（必须强转为通用的 struct sockaddr *）
    // 参数3：地址结构体的实际长度
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("服务端 bind 失败");
        close(server_fd); // 失败时提前关掉打开的套接字
        exit(EXIT_FAILURE);
    }

    // ========== 4. 监听客户端连接 ==========
    // listen 将套接字从“主动”变成“被动监听”模式。
    // 参数1：套接字描述符；参数2：5（指全连接队列的最大长度，表示最多允许 5 个客户端排队等待处理）
    if (listen(server_fd, 5) == -1) {
        perror("服务端 listen 失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("【UDS 服务端】等待客户端连接中...\n");

    // ========== 5. 接受客户端连接 ==========
    // accept 会【阻塞】在这里，直到有一个客户端成功连上来。
    // 一旦成功，它会返回一个全新的 client_fd（此时 server_fd 依然可以继续监听其他客户端）
    // 参数3 client_len 必须传入一个地址长度变量，内核会把客户端的实际地址长度写回这个变量里
    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("服务端 accept 失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("【UDS 服务端】有客户端连接进来了！开始双向通信...\n");

    // ========== 6. 循环处理数据 ==========
    while (1) {
        // 每次循环前，把 buffer 置零，防止上一次读取的数据残留导致打印混乱
        memset(buffer, 0, BUFFER_SIZE);
        
        // 从 client_fd 读取客户端发来的数据。
        // 参数1：套接字 fd；参数2：数据存放的缓冲区；参数3：最大可读字节数（留1字节给 \0）
        num_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        
        // 如果 read 返回值 <= 0：
        // 返回 0 表示对方优雅关闭了连接（执行了 close）
        // 返回 -1 表示发生了读错误
        if (num_read <= 0) { 
            if (num_read == 0) printf("客户端断开连接。\n");
            else perror("读取数据失败");
            break; // 退出 while 循环
        }

        printf("收到客户端消息: %s\n", buffer);

        // 服务端主动回复一个数据给客户端
        const char *response = "服务端已收到，这里是后台服务回复！";
        // write 参数解释：fd(套接字), buf(要发送的数据内存地址), count(发送长度，+1是为了把末尾 \0 也传过去)
        if (write(client_fd, response, strlen(response) + 1) == -1) {
            perror("服务端发送数据失败");
            break;
        }
    }

    // ========== 7. 资源清理 ==========
    close(client_fd); // 关闭客户端连接的分机
    close(server_fd); // 关闭服务端总机
    unlink(SOCKET_PATH); // 删除临时套接字文件
    printf("【UDS 服务端】正常退出，清理资源完成。\n");

    return 0;
}