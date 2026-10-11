
/*
按连接数量和数据频率两个维度，分成四个阶段：

🟢 阶段一：几十台设备（网关内部通信、设备调试）
    典型场景：一台网关连接几十个传感器，或者调试时的本地TCP服务器。
    IO模型：阻塞IO / 多线程。
    触发模式：LT + 阻塞。
    架构模式：单线程循环，或者“一个连接一个线程”。
    核心考量：怎么简单怎么来，代码越少越安全。几十个连接，哪怕用最笨的阻塞读，CPU占用也几乎为零。
    -----总结：“连接数很少时，我直接用阻塞IO或者简单的多线程，没必要引入epoll，增加代码复杂度。”

🟡 阶段二：几百台设备（典型嵌入式网关、边缘计算）
    典型场景：工业物联网网关连接几百个485/CAN节点，或者车载T-Box连接云端。
    IO模型：IO多路复用 + 单Reactor多线程。
    触发模式：LT + 非阻塞循环读（首选，容错高） 或 ET + 非阻塞循环读（追求极致效率）。
    架构模式：主线程用epoll监听所有连接。收到数据后，立刻丢给线程池去解析协议（Modbus、MQTT），解析完再回传给主线程发送。
    核心考量：瓶颈不在网络IO，而在业务解析。所以关键是“让网络线程只干收发，业务逻辑交给线程池，不让耗时的CRC校验阻塞网络循环”。
     -----总结：“几百台连接是嵌入式网关的常态。我会用epoll + 非阻塞，配合单Reactor多线程。主线程只负责收发，数据丢进线程池解析，避免业务卡死网络IO。触发模式优先选LT，安全稳妥。”

🔴 阶段三：上千到上万台设备（云端IoT平台、大型边缘集群）
    典型场景：云端接收上万台设备上报数据，或者大型园区的边缘服务器。
    IO模型：ET边缘触发 + 主从Reactor多线程。
    触发模式：ET + 非阻塞 + 循环读到EAGAIN（标配）。
    架构模式：主线程只负责accept，拿到新连接后，轮询分发给多个子线程。每个子线程有自己的epoll，负责读写和业务。
    核心考量：连接数过万，LT会反复返回就绪fd，导致epoll_wait遍历队列耗时过长。必须用ET减少通知次数。同时单线程扛不住，必须多线程分摊CPU。
     -----总结：“上万连接必须用ET + 非阻塞循环读，减少内核重复通知。架构上必须用主从Reactor，主线程accept，子线程处理读写，充分利用多核CPU。”

🟣 阶段四：上万台设备 + 持续高频数据（工业级/互联网级核心网关）
    典型场景：视频流服务器、金融交易网关、超大规模工业数据采集。
    IO模型：ET + 主从Reactor多进程（+多线程）。
    触发模式：ET + 非阻塞。
    架构模式：多进程 + SO_REUSEPORT。启动多个进程（通常是CPU核数），每个进程都绑定同一个端口。内核负责把新连接均匀分发给各个进程。每个进程内部再跑主从Reactor。
    核心考量：单进程的线程锁竞争和内存分配会成为瓶颈。多进程天生隔离，避免锁开销，利用SO_REUSEPORT让内核做负载均衡（Nginx就是这么干的）。
     -----总结：“如果单机达到上万高频连接，我会采用多进程 + SO_REUSEPORT，让内核在多个进程间做负载均衡，避免线程锁竞争和内存瓶颈。这是Nginx级别的架构。”

    ##  
    连接数	数据频率	触发模式	         IO模型/架构	           核心考量
    几十	任意	    LT + 阻塞	           阻塞IO / 多线程	        简单为主，避免过度设计
    几百	低频突发	LT / ET + 非阻塞	   单Reactor多线程	         网络与业务解耦（线程池）
    上万	低频/突发	ET + 非阻塞	           主从Reactor多线程	     减少内核通知，利用多核
    上万	持续高频	ET + 非阻塞	           多进程 + SO_REUSEPORT	 避免锁竞争，内核负载均衡

》可用交流：
    我之前的项目连接数在几百个左右，属于典型的边缘网关。我采用的是 epoll LT + 非阻塞 + 单Reactor多线程。主线程负责网络收发，用非阻塞循环读保证数据读干净。
    收到数据后，直接丢给线程池去解析Modbus协议和CRC校验，避免耗时的业务逻辑阻塞网络IO。
    如果未来连接数上升到上万，我会把触发模式切换为ET，并将架构升级为主从Reactor，利用多核CPU分摊压力
*/


/*
4.TCP 服务端（阻塞版）

    #define BACKLOG 128
    #define BUF_SIZE 1024

    int tcp_server_start(int port)
    {
        /// 关键点1：创建socket 
        int lfd = socket(AF_INET, SOCK_STREAM, 0);
        if (lfd < 0) { perror("socket"); return -1; }

        //关键点2：端口复用，避免重启时报Address already in use 
        int opt = 1;
        setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        //关键点3：绑定地址，htons/htonl转网络字节序 
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind"); close(lfd); return -1;
        }

        // 关键点4：监听 
        if (listen(lfd, BACKLOG) < 0) {
            perror("listen"); close(lfd); return -1;
        }

        printf("[server] listening on port %d\n", port);

        //关键点5：accept阻塞等待 
        struct sockaddr_in cli;
        socklen_t cli_len = sizeof(cli);
        int cfd = accept(lfd, (struct sockaddr *)&cli, &cli_len);
        if (cfd < 0) { perror("accept"); close(lfd); return -1; }

        printf("[server] client: %s:%d\n",
            inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

        //关键点6：read/write收发数据 
        char buf[BUF_SIZE] = {0};
        ssize_t n = read(cfd, buf, sizeof(buf) - 1);
        if (n > 0) {
            printf("[server] recv: %s\n", buf);
            write(cfd, "hello from server", 17);
        }

        close(cfd);
        close(lfd);
        return 0;

    关键点解析：
        accept 返回新的fd（cfd），用于和该客户端通信；监听fd（lfd）继续监听。
        inet_ntoa 返回静态缓冲区指针，非线程安全，多线程要用 inet_ntop。
        read 返回0表示客户端断开，返回-1要检查 errno。
*/

/*
5.epoll ET模式，此模式必须开非阻塞，，否则数据可能读不全。服务端
#include "epoll_server.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define MAX_EVENTS 1024
#define BUF_SIZE 4096
#define BACKLOG 128

//关键点1：ET模式必须配非阻塞fd 
static int set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int epoll_server_start(int port)
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(lfd); return -1;
    }
    if (listen(lfd, BACKLOG) < 0) {
        perror("listen"); close(lfd); return -1;
    }

    //关键点2：epoll_create1的参数是flags，不是size 
    int epfd = epoll_create1(0);
    if (epfd < 0) { perror("epoll_create1"); close(lfd); return -1; }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

    printf("[epoll] listening on port %d\n", port);

    struct epoll_event events[MAX_EVENTS];

    while (1) {
        // 关键点3：epoll_wait阻塞等待，-1永久阻塞 ，此处阻塞是cpu调度，即没事件发生的时候，主线程挂起。
        //epoll_wait 的 -1 阻塞是事件等待的阻塞，目的是节约 CPU；而 fd 设置 O_NONBLOCK 是数据读写的非阻塞，切记混淆
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == lfd) {
                // 新连接 
                struct sockaddr_in cli;
                socklen_t cli_len = sizeof(cli);
                int cfd = accept(lfd, (struct sockaddr *)&cli, &cli_len);
                if (cfd < 0) continue;

                set_nonblock(cfd);

                // 关键点4：ET边缘触发
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
            } else {
                // 关键点5：ET模式必须循环读到EAGAIN 
                char buf[BUF_SIZE];
                while (1) {
                    ssize_t r = read(fd, buf, sizeof(buf));
                    if (r > 0) {
                        write(fd, buf, r);  // 回显 
                    } else if (r == 0) {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;  // 数据读完了 
                        }
                        if (errno == EINTR) continue;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        break;
                    }
                }
            }
        }
    }

    close(epfd);
    close(lfd);
    return 0;
}

关键点解析：
    ET模式必须配非阻塞fd。否则最后一次 read 会死等，整个服务器卡死。
    epoll_create1 的参数是 flags（0 或 EPOLL_CLOEXEC），不是 size。
    epoll_wait 返回就绪事件数量，-1 表示永久阻塞。
    EPOLLIN | EPOLLET 表示ET模式，只在状态变化时通知一次。
    ET模式必须循环读到 EAGAIN，确保内核缓冲区数据全部读完。
    epoll_ctl(EPOLL_CTL_DEL) 在 close 前调用，避免fd复用后误触发。

*/

