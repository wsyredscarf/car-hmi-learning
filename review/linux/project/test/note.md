>项目有4个模块，单独测每个模块，再联调。

模块	                测试目标	                        怎么测
环形缓冲区	        读写正确、满/空判断、线程安全	     写一个小测试程序，多线程读写
串口初始化	        能打开、配置生效、非阻塞	         用虚拟串口对，stty 查看配置
Modbus状态机	    能正确解析帧、CRC校验	            喂模拟字节流，打印解析结果
整体网关	        串口↔TCP双向透传	                虚拟串口 + TCP客户端联调


1. 测试环形缓冲区（纯逻辑，最简单）
单独写一个 test_ring.c，编译运行：
编译：gcc test_ring.c ring_buffer.c -o test_ring -lpthread
运行：./test_ring
单元测试：给环形缓冲区写单元测试，验证写入、读出、满判断和线程安全
<!--
 #include "ring_buffer.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main() {
    RingBuffer rb;
    rb_init(&rb);
    char w[] = "hello";
    char r[10] = {0};

    int n = rb_write(&rb, w, 5);
    assert(n == 5);
    n = rb_read(&rb, r, 5);
    assert(n == 5);
    assert(strcmp(r, "hello") == 0);

    // 测试满：写满
    char big[5000] = {0};
    n = rb_write(&rb, big, 5000);
    printf("written when full: %d\n", n); // 应该小于5000
    printf("ring buffer test passed\n");
    return 0;
} -->

2. 用虚拟串口测试串口初始化（不用真实硬件）
<!--
Linux 下用 socat 创建一对虚拟串口：
sudo apt install socat   # 如果没装
socat -d -d pty,raw,echo=0 pty,raw,echo=0

会输出两个设备，比如：
/dev/pts/2
/dev/pts/3

修改 main.c，把 uart_open("/dev/ttyS0", ...) 改成 uart_open("/dev/pts/2", ...)。
然后另一个终端用 cat /dev/pts/3 监听。或者用 echo 发送数据：echo -n "test" > /dev/pts/3

如果程序能从串口读到 test，说明串口初始化成功。
也可以用 stty -F /dev/pts/2 -a 查看配置是否变成 8N1、非阻塞等。
-->

3. 测试 Modbus RTU 状态机（喂模拟字节）
写一个 test_modbus.c，模拟一帧 Modbus RTU 数据：
编译运行，观察是否进入 CRC 校验阶段，并打印校验结果。
关键：你可以故意改错 CRC，看程序是否正确报错。

<!--
#include "modbus_parser.h"
#include <stdio.h>

int main() {
    ModbusParser p = {0};
    p.state = MB_STATE_IDLE;

    // 模拟从机地址0x01，功能码0x03，数据长度0x02，数据0x00 0x0A，CRC
    uint8_t frame[] = {0x01, 0x03, 0x02, 0x00, 0x0A, 0x00, 0x00};
    // 注意：最后两个CRC需要根据前5字节计算，这里只是示例

    for (int i = 0; i < sizeof(frame); i++) {
        modbus_parse_byte(&p, frame[i]);
    }
    printf("parse done, state=%d\n", p.state);
    return 0;
}
-->

4. 整体联调（虚拟串口 + TCP 客户端）
这样就验证了“串口→网络”和“网络→串口”的双向透传。
<!--
1.启动虚拟串口对：socat -d -d pty,raw,echo=0 pty,raw,echo=0，得到 /dev/pts/2 和 /dev/pts/3。

2.修改 main.c 里串口设备为 /dev/pts/2。

3.编译运行你的网关程序：./app。

4.用 nc 或 Python 作为 TCP 客户端连接：
bash
nc 127.0.0.1 8888

5.在另一个终端向虚拟串口另一端发数据：
bash
echo -n "hello from uart" > /dev/pts/3

6.观察 TCP 客户端是否收到 hello from uart。
7.反向：在 TCP 客户端输入 hello from tcp，观察 /dev/pts/3 是否收到（用 cat /dev/pts/3 监听）。
-->


// ##补充：
三、调试技巧（面试可提）
1.打印日志：在关键路径加 printf 或 fprintf(stderr, ...)，定位卡在哪。

2.GDB：gdb ./app，break main，run，bt 看堆栈。

3.strace：strace -f ./app 跟踪系统调用，看 read/write/open 是否正常。

4.Valgrind：valgrind --leak-check=full ./app 检查内存泄漏。


你这个项目怎么测试的？”
    我先对每个模块做单元测试：环形缓冲区用多线程读写验证线程安全；串口初始化用 socat 创建虚拟串口对，验证配置；Modbus 状态机用模拟字节流验证解析和 CRC。最后整体联调，用虚拟串口模拟设备，用 TCP 客户端模拟云端，验证双向透传。调试时用 printf、GDB 和 strace 定位问题。









