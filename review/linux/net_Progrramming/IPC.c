/*核心：

【业务需求】
    ↓
【进程/线程】← 承载业务逻辑
    ↓
【通信方式】
    ├── 本机进程间：IPC（管道/消息队列/共享内存/信号量/信号）
    ├── 本机线程间：全局变量 + 互斥锁/条件变量
    └── 跨主机/跨设备：网络编程（socket）
            ↓
        【传输层】TCP / UDP
            ↓
        【IO 模型】阻塞 / 非阻塞 / IO 多路复用（select/poll/epoll）
            ↓
        【网络接口】eth0 / wlan0 / wwan0（4G）


系统编程的核心是进程和线程，它们是业务的执行体。进程间通信用 IPC，线程间通信用共享内存加锁。
网络编程是系统编程的一个分支，本质是让进程通过 socket 使用内核网络栈。选择 TCP 还是 UDP 看业务需求，是否需要可靠传输。
如果服务器要同时处理多个客户端，就用 IO 多路复用，Linux 下首选 epoll。
*/


/*
-------------------------------------------------------------------------网络编程
TCP/UDP  核心区别:
维度	    TCP                     	UDP
连接	    面向连接（三次握手）	      无连接
可靠性	    可靠，有确认、重传、排序	   不可靠，不保证到达
传输方式	字节流	                      数据报
首部开销	20字节	                      8字节
速度	   慢（可靠机制开销）	           快
场景	   文件传输、HTTP、MQTT	          视频直播、DNS、广播
口述：“TCP 面向连接、可靠、字节流，适合对数据完整性要求高的场景；
      UDP 无连接、不可靠、数据报，适合实时性要求高、能容忍丢包的场景，比如视频流和 DNS。”

##
socket核心步骤：
// 1. 创建 socket
    // AF_INET: IPv4, SOCK_STREAM: TCP
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) { perror("socket"); return -1; }

// 2. 设置端口复用（关键：避免重启时 "Address already in use"）
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

// 3. 绑定地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);           // 端口转网络字节序
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网卡
    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return -1;
    }

// 4. 监听（backlog 是已完成连接队列 + 未完成连接队列的最大值）
    if (listen(lfd, 128) < 0) { perror("listen"); return -1; }

// 5. 接受连接（阻塞直到有客户端连接）
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int cfd = accept(lfd, (struct sockaddr *)&cli_addr, &cli_len);
    if (cfd < 0) { perror("accept"); return -1; }

// 6.数据处理，打印客户端 IP 和端口，
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));
    printf("client: %s:%d\n", ip, ntohs(cli_addr.sin_port));

    char buf[1024];
    ssize_t n = read(cfd, buf, sizeof(buf));
    if (n > 0) {
        buf[n] = '\0';
        printf("recv: %s\n", buf);
        write(cfd, "hello client", 12);
    }

// 7. 关闭
    close(cfd);
    close(lfd);

关键点：
socket() 返回文件描述符，本质是内核中的一个 socket 结构体。
setsockopt 设置 SO_REUSEADDR 是工程必备，否则服务器重启会报“地址已被占用”。
htons/htonl 把主机字节序转网络字节序（大端）。
listen 的 backlog 不是连接数上限，是等待队列长度。
accept 返回的是新的 fd，用于和该客户端通信；监听 fd 继续监听。


##
TCP 三次握手 / 四次挥手
三次握手
客户端 → 服务端：SYN (seq=x)              【我想连接】
服务端 → 客户端：SYN + ACK (seq=y, ack=x+1) 【同意，我也想连接】
客户端 → 服务端：ACK (ack=y+1)             【确认】
为什么三次？
“两次只能确认客户端→服务端的通道，无法确认服务端→客户端的通道。
三次才能双方都确认彼此的收发能力正常，同时防止旧的连接请求突然到达服务端造成资源浪费。”

四次挥手
主动方 → 被动方：FIN (seq=u)              【我要关闭】
被动方 → 主动方：ACK (ack=u+1)            【收到，但我可能还有数据要发】
被动方 → 主动方：FIN (seq=w)              【我也关了】
主动方 → 被动方：ACK (ack=w+1)            【确认】
为什么四次？
“TCP 是全双工的，关闭需要两个方向分别关闭。被动方收到 FIN 后可能还有数据要发，所以 ACK 和 FIN 不能合并，因此是四次。”

TCP 粘包 / 半包（高频）
原因：TCP 是字节流，没有消息边界。发送方多次 send 可能被合并，一次 send 可能被拆分。
解决三种方案：
定长：每条消息固定长度。
分隔符：如 \r\n，读到分隔符算一帧。
长度字段：帧头带长度，先读长度，再读数据。
口述：“我一般用长度字段+状态机。状态分四步：等待帧头、读长度、读数据、CRC校验。用环形缓冲区存半包数据，下次收到数据拼上去继续解析。”


##
IO 多路复用：select / poll / epoll
维度	         select	  poll	  epoll
数据结构	      位图	   链表	    红黑树 + 就绪链表
最大连接	      1024	  无上限	无上限
时间复杂度	      O(n)	  O(n)	   O(1)
触发模式	      LT	  LT	   LT + ET
跨平台	          好	 一般	 Linux 专属
LT vs ET（必须能说）：
LT（水平触发）：只要缓冲区有数据，epoll_wait 就一直通知。安全，但可能重复通知。
ET（边缘触发）：只有状态变化时通知一次。必须搭配非阻塞 IO，且必须循环读到 EAGAIN，否则会丢数据。
*/



/*
------------------------------------------------------------IPC 进程间通信

方式	        方向	速度	场景	      特点
匿名管道	     单向	中	    父子进程	  pipe()，简单
命名管道FIFO	 单向	中	    无亲缘进程	   mkfifo，文件系统可见
消息队列	     双向	中	    结构化消息	  有边界，可分类
共享内存	     双向	最快	大数据量	  需配信号量同步
信号量	          -	     -	   同步/互斥	 计数器，不传数据
信号	        单向	快	   事件通知	      异步，信息量少
口述：“共享内存最快，因为数据直接在内存里共享，不需要内核拷贝。但它需要信号量来同步。管道简单但只能单向，消息队列有边界，信号用于异步通知。”


##
管道（pipe）：

int fd[2];
pipe(fd);  // fd[0] 读端, fd[1] 写端
pid_t pid = fork();
if (pid == 0) {
    // 子进程写
    close(fd[0]);        // 关闭不用的读端
    write(fd[1], "hello", 5);
    close(fd[1]);
} else {
    // 父进程读
    close(fd[1]);        // 关闭不用的写端
    char buf[32];
    read(fd[0], buf, sizeof(buf));
    close(fd[0]);
}
关键点：
管道是半双工，同一时间只能单向。
读写端都关闭后，读端返回 0。
写端关闭，读端 read 返回 0；读端关闭，写端 write 触发 SIGPIPE。


##
命名管道（FIFO）：

mkfifo /tmp/myfifo
// 写进程
int fd = open("/tmp/myfifo", O_WRONLY);
write(fd, "hello", 5);
close(fd);

// 读进程
int fd = open("/tmp/myfifo", O_RDONLY);
read(fd, buf, sizeof(buf));
close(fd);
区别：FIFO 在文件系统有名字，任意进程都能打开；匿名管道只能用于有亲缘关系的进程。


##
共享内存（最快）：

// 1. 创建共享内存
int shmid = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0666);
// 2. 映射到进程地址空间
char *addr = shmat(shmid, NULL, 0);
// 3. 读写
strcpy(addr, "hello shared memory");
printf("%s\n", addr);
// 4. 解除映射
shmdt(addr);
// 5. 删除共享内存
shmctl(shmid, IPC_RMID, NULL);

关键点：
共享内存本身不提供同步机制，多个进程同时写会出问题。
必须配合信号量或互斥锁使用。
shmdt 只是解除映射，shmctl(IPC_RMID) 才是真正删除。


##
消息队列：

struct msgbuf {
    long mtype;      // 消息类型，必须 > 0
    char mtext[128]; // 消息内容
};

// 创建
int msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

// 发送
struct msgbuf msg;
msg.mtype = 1;
strcpy(msg.mtext, "hello");
msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0);

// 接收（按类型）
struct msgbuf rcv;
msgrcv(msqid, &rcv, sizeof(rcv.mtext), 1, 0);

// 删除
msgctl(msqid, IPC_RMID, NULL);
特点：消息有边界，支持按类型读取，比管道灵活。消息队列
c
#include <sys/msg.h>

struct msgbuf {
    long mtype;      // 消息类型，必须 > 0
    char mtext[128]; // 消息内容
};

// 创建
int msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

// 发送
struct msgbuf msg;
msg.mtype = 1;
strcpy(msg.mtext, "hello");
msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0);

// 接收（按类型）
struct msgbuf rcv;
msgrcv(msqid, &rcv, sizeof(rcv.mtext), 1, 0);

// 删除
msgctl(msqid, IPC_RMID, NULL);
特点：消息有边界，支持按类型读取，比管道灵活。


##
信号量（同步）：

sem_t sem;
sem_init(&sem, 1, 1);   // 1 表示进程间共享，初值 1

sem_wait(&sem);         // P 操作，减 1，为 0 时阻塞
// 临界区
sem_post(&sem);         // V 操作，加 1

sem_destroy(&sem);
互斥锁 vs 信号量：

互斥锁有 所有权，谁锁谁解。
信号量无 所有权，任何进程都能 post。


##
信号（signal）：

void handler(int sig) {
    printf("got signal %d\n", sig);
}
signal(SIGINT, handler);   // Ctrl+C
signal(SIGPIPE, SIG_IGN);  // 忽略管道破裂
signal(SIGCHLD, SIG_IGN);  // 自动回收子进程
常用信号：
SIGINT：Ctrl+C。
SIGKILL：强制杀死，不可捕获。
SIGPIPE：写已关闭的 socket 或管道。
SIGCHLD：子进程退出。

*/
/*
进程间通信主要有六种：管道、命名管道、消息队列、共享内存、信号量、信号。
管道简单但只能单向，且只能用于亲缘进程；
命名管道在文件系统有名字，任意进程可用；
消息队列有边界、支持按类型读取；
共享内存最快，因为数据直接共享，不用内核拷贝，但需要信号量同步；
信号量主要用于同步和互斥；
信号用于异步事件通知，信息量少。实际项目中，共享内存+信号量用于大数据传输，
socket 用于跨主机通信，管道用于父子进程间简单通信。
*/