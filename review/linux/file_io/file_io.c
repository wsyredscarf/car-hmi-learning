/*
文件 IO 与 标准 IO 的区别（理清概念）：
    对比维度	            文件IO（系统调用）	                    标准IO（C库函数）
    接口	                open/read/write/close/lseek	          fopen/fread/fwrite/fclose/fseek
    返回类型	             int 文件描述符（fd）	                FILE * 文件指针
    用户态缓冲	            无（直接进内核）	                    有（全缓冲/行缓冲）
    适用场景	            设备文件（串口、I2C、SPI）、Socket	     普通文本文件、配置文件、日志
    跨平台	                只限 Unix/Linux	                        Windows/Linux 通用
    总结：“fopen 底层也是调用 open 实现的。在嵌入式 Linux 中，操作设备文件必须用 open，
        因为需要传 O_NOCTTY、O_NONBLOCK 等底层标志位，且设备文件不支持标准 IO 的缓冲机制。”
*/

//代码：
/*
##拆解：
    open：打开/创建文件
        #include <fcntl.h>
        #include <unistd.h>
        #include <stdio.h>

        int fd = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open");   // 打印错误信息
            return -1;
        }
        关键点：
            返回文件描述符（fd），从 3 开始递增（0、1、2 分别被 stdin、stdout、stderr 占用）。
            O_RDONLY、O_WRONLY、O_RDWR 必须选一个。
            常用标志：O_CREAT（不存在则创建）、O_TRUNC（清空）、O_APPEND（追加）、O_NONBLOCK（非阻塞）。
            权限位 0644 受 umask 影响（实际权限 = mode & ~umask）。

    read：读取文件
        char buf[128];
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            perror("read");
        } else if (n == 0) {
            printf("end of file\n");  // 读到文件末尾
        } else {
            buf[n] = '\0';
            printf("read %ld bytes: %s\n", n, buf);
        }
        关键点：
            返回值 n：实际读取字节数。
            n == 0：表示读到文件末尾（EOF）。
            n == -1：出错。需检查 errno：
            EAGAIN / EWOULDBLOCK：非阻塞模式下暂时没数据。
            EINTR：被信号中断，需重试。

    write：写入文件
        const char *msg = "hello linux";
        ssize_t n = write(fd, msg, strlen(msg));
        if (n < 0) {
            perror("write");
        }
        关键点：
            返回值 n：实际写入字节数。可能小于请求字节数（如写磁盘满、信号中断），必须循环写直到全部写完！

    lseek：移动读写偏移量
        off_t offset = lseek(fd, 0, SEEK_SET);   // 从头开始
        offset = lseek(fd, 0, SEEK_END);         // 移动到末尾（常用于获取文件大小）
        offset = lseek(fd, -10, SEEK_END);       // 倒数第10个字节
        关键点：
            SEEK_SET：从文件头开始。
            SEEK_CUR：从当前位置开始。
            SEEK_END：从文件末尾开始。
            返回新的偏移量，失败返回 -1。
            设备文件（如串口）通常不支持 lseek。

    close：关闭文件
        close(fd);
        关键点：
            不关闭会导致文件描述符泄漏，最终进程耗尽 fd（默认上限 1024）
            关闭后，fd 可被内核回收给下一次 open 使用。

##整体：
    #include <fcntl.h>    // open, O_RDONLY, O_WRONLY, O_CREAT
    #include <unistd.h>   // read, write, close
    #include <stdio.h>    // perror
    #include <string.h>   // 

    int main() {
        // 1. 以只读方式打开源文件
        int src = open("src.txt", O_RDONLY);
        if (src < 0) {            // 判断文件是否存在或权限不足
            perror("open src");   // 打印错误信息（如：open src: No such file or directory）
            return -1;            // 打开失败，直接退出
        }

        // 2. 以只写、创建、清空方式打开目标文件，权限 0644（rw-r--r--）
        int dst = open("dst.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (dst < 0) {            // 判断磁盘满、权限不足等
            perror("open dst");
            close(src);           // 【关键点1】打开目标文件失败，必须关闭已经打开的源文件，防止 fd 泄漏
            return -1;
        }

        // 3. 定义缓冲区，每次读取 1KB
        char buf[1024];
        ssize_t n;                // ssize_t 是有符号整型，用于接收 read 的返回值

        // 4. 循环读取源文件内容，直到读到文件末尾（返回0）
        // 【关键点2】read 的返回值：
        //   > 0: 实际读取的字节数
        //   = 0: 文件末尾（EOF），退出循环
        //   < 0: 出错，需检查 errno
        while ((n = read(src, buf, sizeof(buf))) > 0) {
            ssize_t written = 0;  // 记录已写入的字节数

            // 5. 【关键点3】write 不能保证一次写入全部数据！
            //    可能因磁盘满、信号中断等原因只写入部分。
            //    必须用循环确保所有已读到的 n 字节都成功写入。
            while (written < n) {
                ssize_t w = write(dst, buf + written, n - written);
                if (w < 0) {      // 写入出错
                    perror("write");
                    close(src);   // 出错时关闭所有已打开的 fd，防止泄漏
                    close(dst);
                    return -1;
                }
                written += w;     // 累加已写入字节数
            }
        }

        // 6. 检查 read 最终是否出错（返回 -1）
        if (n < 0) {
            perror("read");       // 打印读取错误信息
        }

        // 7. 【关键点4】关闭文件描述符
        //    必须成对 close，否则 fd 泄漏，进程耗尽资源后无法再打开新文件
        close(src);
        close(dst);

        return 0;
    }
*/
    //关键点：
        // 手撕代码，4个关键点必须边写边说出来：
        // 资源释放：任何一步 open 失败，都要 close 之前已经打开成功的 fd。有没有这个严谨性。
        // read 返回值判断：> 0 成功，= 0 文件末尾，< 0 出错。不能只写 != 0。
        // write 的部分写问题：内层 while(written < n) 循环是加分项！90%的人会忽略 write 可能写不全，直接写一句 write(dst, buf, n) 就完了，这是严重的边界漏洞。
        // close 泄漏：无论哪个环节失败，都要确保已打开的 fd 被关闭。


/*
//为什么 write 需要循环，而 read 不需要循环（用 while((n=read(...))>0) 就够）？
一、为什么 write 需要循环（关键考点）
    write 的返回值是实际写入的字节数。它可能小于你请求写入的字节数 n。原因包括：
    内核缓冲区满：非阻塞模式下，内核发送缓冲区满了，只能写进去一部分。
    被信号中断：write 被信号打断，返回已写入的部分。
    磁盘空间不足：写普通文件时，写了一半发现磁盘满了。
    管道/FIFO 缓冲区满（4kB）：写入管道时，如果读端没及时读，只能写一部分。
    所以，如果只是简单调用一次 write(dst, buf, n)，你可能只写了一半的数据，剩下的就丢了。必须用循环确保 written 累加到等于 n。这是面试官最想看到的边界处理意识。

二、 为什么 read 不需要内层循环？
    read 返回的是实际读到的字节数，这个值已经是你本次能拿到的所有数据了。
    你调用 read(fd, buf, 1024)，它返回 n。这个 n 就是实际读到的字节数，可能少于 1024，也可能等于 0（EOF）。
    你直接把 buf[0..n-1] 拿去处理就行了，不需要“再读一次凑够 1024”。
    外层的 while((n = read(...)) > 0) 是为了读取整个文件直到 EOF，不是为了“读满缓冲区”。
    ⚠️ 注意：在 Socket 和串口场景下，read 也会“读不全”
    如果是要读取固定长度的数据（比如网络协议里读 4 字节的长度字段），同样需要写一个 read_full 循环，直到读满 4 字节或者超时。这正好是网络编程的重点。
    
三、 面试标准回答（这个）
    “write 调用可能因为内核缓冲区满、被信号中断等原因只写入部分数据，所以必须用循环确保所有字节都写入。
    而 read 返回的是实际读到的字节数，外层循环只是用来读到文件末尾 EOF。但在需要读取固定长度数据时，read 同样需要循环读满。”
    */


/*
补充：

读写分类（面试分类必备）
1. 按“是否需要缓冲区”分类
    系统调用 IO（无缓冲）：open/read/write。每次调用直接进内核，频繁小数据读写效率低。
    标准 IO（有缓冲）：fopen/fread/fwrite。用户态有缓冲区，攒够了一批再调用系统调用，效率高。

2. 按“阻塞行为”分类
    阻塞 IO：默认模式。read 没有数据就死等，直到有数据或出错。
    非阻塞 IO：open 时加 O_NONBLOCK。read 没有数据立刻返回 -1，并设置 errno = EAGAIN。

3. 按“数据流向”分类
    读：read，从内核缓冲区拷贝到用户缓冲区。
    写：write，从用户缓冲区拷贝到内核缓冲区。

4. 按“文件类型”分类（嵌入式重点）
    普通文件：test.txt、config.json。支持 lseek。
    设备文件：串口 /dev/ttyS0、I2C /dev/i2c-1、SPI /dev/spidev0.0。必须用 open，通常配 O_NOCTTY | O_NONBLOCK，不支持 lseek。
    管道/FIFO：read 返回 0 表示写端全部关闭。
    Socket：用 socket 创建，用 read/write 或 send/recv 读写。
*/