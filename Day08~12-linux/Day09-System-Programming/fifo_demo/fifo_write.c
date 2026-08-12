#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define FIFO_PATH "./myfifo"

int main(void)
{
    // // 创建有名管道文件，如果已经存在 mkfifo会报错，忽略即可,也可以直接创建指令：mkfifo 文件名
    // mkfifo(FIFO_PATH, 0664);//文件权限

    // 以只写方式打开FIFO
    int fd = open(FIFO_PATH, O_WRONLY);
    if(fd < 0)
    {
        perror("open fifo write fail");
        return 1;
    }

    char buf[] = "这是有名管道发送的数据，无亲缘进程也能通信";
    printf("写进程：准备发送数据\n");

    write(fd, buf, strlen(buf)+1);
    printf("写进程：发送完成\n");

    close(fd);

    return 0;
}