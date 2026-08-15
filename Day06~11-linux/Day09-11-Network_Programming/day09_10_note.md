# Day08 学习日志

    系统编程
        |——网络编程
            |——本地通信
                |——域套接字（Unix Domain Socket, UDS）、消息队列、管道、共享内存、信号量、信号
            |——网络通信
                |——TCP/UDP
            |——IO多路复用(单进程监听多客户端)
                |——select
                |——poll
                |——epoll


## 学习内容
    *域套接字（Unix Domain Socket, UDS）是一种在同一台主机上的进程之间进行双向通信的机制。它借用了网络套接字（socket）的编程接口。以用熟悉的 bind 、 listen 、 accept 、 connect 、 send 、 recv 等函数来操作，但数据不走网络协议 栈，而是直接在内核中通过文件系统路径或抽象命名空间进行拷贝或共享。这意味着它避免了网络协议（如TCP/IP）的封装、校验和、路由等开销，性能可以逼近甚至超越共享内存，同时又提供了面向字节流或数据报的可靠通信模型，比管道和消息队列更灵活。
        优点：高性能、高可靠性和便捷性的本地通信方案
        使用场景：任何网络编程的起点。


    *TCP（传输控制协议）  :是一种面向连接的、可靠的网络传输协议。不同计算机上的进程可以通过IP地址和端口号建立TCP连接，进行数据的可靠传输（3握4挥）。
        优点：
            1、可靠性极高、面向连接：必须经过“三次握手”建立连接才能通信。
            2、可靠传输：保证数据顺序，不丢包、不乱序（如果丢了会自动重传）
            3、流量控制与拥塞控制：会根据网络状况自动调节发送速度，防止网络瘫痪。
        缺点（代价大）：
            1、速度较慢、延迟高：因为需要握手、校验、应答和重传，消耗更多系统资源。
            2、粘包问题：数据是流式的，没有消息边界，开发时需自行处理分包（比如协议头加长度字段）。
        核心使用场景（车载必用）：
            远程车机 OTA 升级、车载 Web API 接口、文件传输、UDS 诊断（部分基于 TCP）、车内核心业务指令控制。


    *UDP（用户数据报协议）:是无连接的、不可靠的网络传输协议。虽然不保证数据的可靠传输，但由于没有连接建立和维护的开销，传输速度快，实时性好。
        优点（速度极快）：
            1、无连接：不需要握手，想发就发。
            2、头部开销极小：只有 8 个字节。
            3、延迟极低：不保证顺序，不保证可靠，不重传
        缺点（丢包不负责）：
            1、不可靠：网络拥塞时丢包了，协议层不管，需要应用层自己处理。
            2、数据可能会乱序：先发的包可能最后才到。
        核心使用场景（车载必用）：
            车载娱乐音视频流媒体（RTSP/RTP）、车机内部多传感器高频数据广播（比如每秒 100 次陀螺仪数据，允许偶尔丢几帧）、DNS 域名解析。


    *select IO多路复用 :select靠的是一个叫fd_set的位图（Bitmask）全量集合来管理文件描述符。位图的大小，是C语言标准库的头文件里写死的，宏定义叫做FD_SETSIZE。大多数Linux发行版中，被定义为1024。
        优点：
            跨平台能力最强：不仅Linux能用，老旧的Unix，甚至Windows的Winsock也有完全一样的API。做一套兼容 Windows/Linux 的代码设计，就选这个。
        缺点（严重缺陷）：
            1、硬编码上限：最大只能监听 1024 个文件描述符（受 FD_SETSIZE 限制）。
            2、性能差（O(n) 遍历）：每次调用都要把所有的 fd 从用户态拷到内核态，并遍历所有 fd 检查谁有事件（哪怕只有 1 个有事件，也要遍历 1024 个）。
        使用场景：
            连接数极少（< 1024）、跨平台要求极高、或者用来写教学演示代码。


    *poll  IO多路复用 :poll靠遍历全量数组来管理文件描述符，修改 ulimit（ Linux Shell 提供的一个“系统资源配额控制命令”）65535，来扩大连接数
        优点：
            1、破除 1024 硬限制：没有数量限制（受限于系统 ulimit -n）。
            2、events 和 revents 分离，不需要像 select 那样每次重置集合。
        缺点（依然有缺陷）：
            依然 O(n) 遍历：虽然有进步，但调用 poll 时，依然需要把所有 socket 组成的数组从用户态拷贝到内核态，依然要遍历整个数组找活跃的 socket。
        使用场景：
            需要连接数几千、不需要极致性能、且无法用 epoll（比如早期嵌入式 Linux 内核版本过低）的项目。


    *epoll IO多路复用 ：epoll靠内核回调机制，通过mmap内存映射，内核态与用户态共享内存，把“内核维护的 socket 事件表”这块内存，直接共享给了用户态程序
    添加删除时，不需要把数据从你的程序拷贝到内核，而是直接修改这块内存，属于“零拷贝”的机制。（也需要修改 ulimit来扩大连接数量）
        优点（性能断崖式领先）：
            1、无上限：支持百万级并发，仅受系统物理内存限制。
            2、O(1) 复杂度：利用了内核回调机制，epoll_wait 返回的一定是有事件的那几个，完全不需要遍历没有事件的 socket。
            3、mmap 零拷贝：用户态和内核态共享一块内存，添加/删除 fd 时不用拷贝数据。
            4、支持 LT（水平触发）和 ET（边缘触发）：LT 模式安全，适合新手；ET 模式极致高效，适合高并发。
        缺点：
            仅限 Linux 环境，不能跨 Windows 等平台（但车载座舱的 SoC 基本都是 Linux/QNX，所以这根本不是缺点）。
        使用场景：
            智能座舱网络后台服务、车载 T-Box 网关、高并发车联网服务器、各类大型框架（如 Nginx、Redis 底层正是 epoll）。只要你在 Linux 下做服务器，闭眼选 epoll 即可。

    

## 重要知识点总结
    *域套接字：
        使用流程：
            服务端：
                前置要求：创建tmp文件标识符通道、创建文件描述符（int类型）、创建存储服务器和客户端地址的结构体（sockaddr_un()）、长度socklen_t()
                1.创建套接字         fd=sockert(AF_INET,SOCK_STREAM,0)
                2.清理残留套接字文件  unlink();
                3.将文件路径拷贝     sun_path 成员中（用strncpy()）
                   然后绑定          bind(fd,(struct sockaddr *),sizeof(sockaddr));
                4.监听网络           listen(fd,10);
                5.接受客户端的连接请求 accept(fd,(struct sockaddr * client),socklen_t * len)
                6. 循环处理 收发消息   read(fd,buf,len);write(fd,”hello”,6)
             客户端：
                1.创建套接字
                2. 连接服务器   connect(cfd,struct sockaddr * seraddr,sizeof(seraddr))
                3.收发消息
                4.关闭套接字
    *TCP
        使用流程：
            服务端：
                前置要求：服务端绑定端口号（创建）、创建文件描述符（int类型）、创建存储服务器和客户端地址的结构体（sockaddr_in()）、
                1.创建套接字    fd=sockert(AF_INET,SOCK_STREAM,0)
                2.绑定ip和端口  bind(fd,(struct sockaddr *),sizeof(sockaddr));
                3.监听网络      listen(fd,10);
                4.接受客户端的连接请求 accept(fd,(struct sockaddr * client),socklen_t * len)
                5. 收发消息      read(fd,buf,len);write(fd,”hello”,6)
                6.关闭套接字     close(fd);
             客户端：
                1.创建套接字
                    配置连接测试（172.0.0.1）htons把主机字节序转成网络字节序
                2. 连接服务器   connect(cfd,struct sockaddr * seraddr,sizeof(seraddr))
                3.收发消息    send、recv
                4.关闭套接字  close（fd）

    *UDP（不需要链接）
        使用流程：
            服务端：
                前置要求：服务端绑定端口号（创建）
                1.创建套接字    fd=sockert(AF_INET,SOCK_STREAM,0)
                2.绑定ip和端口  bind(fd,(struct sockaddr *),sizeof(sockaddr));
                3.监听网络      listen(fd,10);
                5. 收发消息      read(fd,buf,len);write(fd,”hello”,6)
                6.关闭套接字     close(fd);
             客户端：
                1.创建套接字
                    配置连接测试（172.0.0.1）htons把主机字节序转成网络字节序
                2. 连接服务器   connect(cfd,struct sockaddr * seraddr,sizeof(seraddr))
                3.收发消息     recvfrom  、sendto
                4.关闭套接字
    *select（IO多路复用）
        使用流程：
                前置要求：服务端绑定端口号（创建）、创建文件描述符（int类型）、创建存储服务器和客户端地址的结构体（sockaddr_in()）、读描述符集合、临时集合
                1.创建套接字    fd=sockert(AF_INET,SOCK_STREAM,0)
                2.绑定ip和端口  bind(fd,(struct sockaddr *),sizeof(sockaddr));
                3.监听网络      listen(fd,10);
                4.初始化集合，   `readfds`把监听套接字 `server_fd` 放进去，FD_ZERO、FD_SET。
                5.检测是否有可读的集合   select，遍历集合内的所有文件描述符是否有可读的集合
                6.接受新连接请求  accept(fd,(struct sockaddr * client),socklen_t * len)
                5.收发消息      read(fd,buf,len);write(fd,”hello”,6)
                6.关闭套接字     close(fd);
     *poll（IO多路复用）
                前置要求：服务端绑定端口号（创建）、创建文件描述符（int类型）、创建存储服务器和客户端地址的结构体sockaddr_in()、结构体数组管理描述符
                1.创建套接字    fd=sockert(AF_INET,SOCK_STREAM,0)
                2.绑定ip和端口  bind(fd,(struct sockaddr *),sizeof(sockaddr));
                3.监听网络      listen(fd,10);
                4.核心初始化    将监听 socket 放入 pollfd 数组第一位
                5.创建poll      int poll(struct pollfd *fds, nfds_t nfds, int timeout);结构体数组指针、实际监控的数量、阻塞和非阻塞
                6.遍历数组      pollfd 数组（性能退化点：依然要遍历 nfds 个元素）
                6.接受新连接请求 /处理数据       accept(fd,(struct sockaddr * client),socklen_t * len) /recv(),send()
                6.关闭套接字     close(fd);

     *epoll（IO多路复用）
                前置要求：服务端绑定端口号（创建）、创建文件描述符（int类型）、创建存储服务器和客户端地址的结构体sockaddr_in()、结构体数组管理描述符
                1.创建套接字    fd=sockert(AF_INET,SOCK_STREAM,0)
                2.绑定ip和端口  bind(fd,(struct sockaddr *),sizeof(sockaddr));
                3.监听网络      listen(fd,10);
                4.创建 epoll      epoll_fd = epoll_create1(0);
                5.创建poll      将监听套接字加入 epoll 关心的事件中、使用 ET 边缘触发模式！
                6.设为非阻塞（ET模式下必须做的！）   set_nonblocking(server_fd);
                7.循环核心函数：  int epoll_wait(...)，内核通过红黑树保存 fd，通过回调机制通知。内核只返回发生了事件的 fd。
                6.接受新连接请求 /处理数据       accept(fd,(struct sockaddr * client),socklen_t * len) /recv(),send()
                6.关闭套接字     close(fd);


## 代码练习
    .c文件

## 补充：
1、Unix域套接字是一种在同一台主机上进程间进行高效、可靠通信的机制，通过文件系统中的特殊文件标识实现数据传输。
    基本概念：
        Unix域套接字（UDS）是一种用于同一台机器上进程间通信（IPC）的机制，它不依赖网络协议栈，而是直接在文件系统中创建一个特殊类型的文件（tmp临时文件）作为通信通道 
        。与TCP或UDP套接字类似，Unix域套接字提供面向流（SOCK_STREAM）和面向数据报（SOCK_DGRAM）两种接口，但仅限于本机进程间通信 

    工作原理：
        Unix域套接字通过文件系统路径标识通信端点。进程通过以下步骤使用Unix域套接字：   
            创建套接字：调用socket(AF_UNIX, type, 0)返回一个文件描述符，用于后续操作 
            绑定套接字：将套接字绑定到文件系统中的路径，路径可以是已存在或新创建的文件 
            监听和连接：服务器进程可调用listen监听连接，客户端通过connect连接到指定路径的套接字 
            数据传输：进程通过读写套接字文件描述符进行数据交换，数据直接在内存中复制，无需网络协议处理 

    优势：
        高效：无需添加网络报头、计算校验和或维护序号，数据直接在内存中复制，效率通常高于TCP套接字 。
        可靠：提供可靠的数据传输，消息不会丢失或顺序错乱 。
        灵活：支持命名套接字和非命名套接字（通过socketpair创建互联套接字），可实现全双工通信 
    
    注意点：路径命名方式可选文件系统路径或抽象命名空间、确保Socket文件权限设置合理，防止安全风险、仅限同一主机使用，不能跨机器通信

    应用场景：
        1.本地高频数据交互：如音视频处理进程间传输原始数据
        2.数据库本地连接：  MySQL等在localhost下默认使用UDS以提升性能
        3.插件或模块间通信：无需网络开销，快速交换数据
        4.也可用于实现高效的本地IPC机制，替代管道或共享内存。

2、五种IO模型
任何涉及到计算机核心（CPU和内存）与其他设备间的数据转移的过程就是IO,应用程序发起一次IO访问分为两个阶段:
    IO调用阶段：应用程序向内核发起系统调用。
    IO执行阶段：内核执行IO操作并返回。
    数据准备阶段：内核等待IO设备准备好数据
    数据拷贝阶段：将数据从内核缓冲区拷贝到用户空间缓冲区

    *阻塞IO: 应用程序发起一个系统调用，然后一直阻塞，等待内核把数据处理完毕复制到用户空间，返回成功标志后，应用程序才会继续处理数据
            优点:模型简单，实现难度低，适用于并发量较小的应用开发。
            缺点:IO调用阶段和IO执行阶段都会阻塞。

    *非阻塞IO: 应用进程需要不断询问内核数据是否就绪，在内核数据还未就绪时，应用进程还可以做其他事情。
            优点:模型简单，实现难度低;与阻塞IO模型对比，它在等待数据报的过程中，进程并没有阻塞，它可以做其他的事情。
            缺点:轮询发送 recvform，消耗CPU 资源。

    *IO复用:多个进程的IO注册到一个复用器（select）上，select 会监听所有注册进来的IO。如果内核的数据报没有准备好，调用select 的进程将会被阻塞，而当任一个IO在内核缓冲区中有数据，select调用就会返回可读条件，然后进程再进行recvform系统调用，内核将数据拷贝到用户空间。注意这个过程是阻塞的。
            优点:适用于高并发应用程序。
            缺点:模型复杂，实现、开发难度较大。

    *信号驱动IO://

    *异步IO://


## 遇到问题 & 解决方案
*座舱多进程本地通信为什么优先 Unix 域套接字，不用 TCP？
    答：Unix 域不走 TCP/IP 协议栈，仅内核内存拷贝，延迟更低、CPU 占用更少，适合和后台高频交互。
*共享内存速度最快为什么不能单独使用？
    答：多进程同时读写会产生数据竞争，无内置同步机制，必须搭配信号量 / 信号完成互斥访问。
*sigaction 和 signal 区别？
    答：signal 捕获信号后会重置处理函数，不可重入；sigaction 稳定支持 SA_RESTART、SA_SIGINFO，工程开发标准 API。

*setsockopt()函数作用是什么？为什么要加？还有其他作用吗？
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    答: setsockopt 是所有网络程序穿的常态配置, 管“怎么配置硬件网卡和端口”，允许服务器在崩溃、或者按 Ctrl+C 强制退出后，立刻（几秒内）重启，而不报 Address already in use（地址已被占用）的错误。
    答：防止服务器关闭后，立刻重启后(代码参数填SO_REUSEADDR)解决 TCP 的 TIME_WAIT 状态。
    答：
        SO_RCVBUF / SO_SNDBUF：调整 socket 的发送和接收缓冲区大小（处理大量数据时必须调大）。
        TCP_NODELAY：禁用 Nagle 算法。车载 TCP 通信，有时要求毫秒级超低延迟，就需要开启这个选项（1 表示禁用，0 表示启用）。
        SO_KEEPALIVE：开启 TCP 保活探测。如果网络异常断开了，两端都没有数据收发，用这个选项让内核定时发送探测包，帮及时发现连接断没断。

    本地套接字（unix域套接字socket，因为使用的时临时文件tmp/路径下的）使用unlink即可清理残留文件。
    其余的配置网络端口的都可以使用
****
域套接字和epoll为重点：
性能、高并发、数据处理效率方面

待补充