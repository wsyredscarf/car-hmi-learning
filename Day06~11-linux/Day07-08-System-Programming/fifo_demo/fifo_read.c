#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define FIFO_PATH "./myfifo"

int main(void)
{

    int fd = open(FIFO_PATH, O_RDONLY);//参数固定
    if(fd < 0)
    {
        perror("open fifo read fail");
        return 1;
    }

    char recv_buf[128] = {0};
    printf("读进程：等待接收数据...\n");

    ssize_t ret = read(fd, recv_buf, sizeof(recv_buf));
    printf("读进程：收到%zd字节:内容:%s\n", ret, recv_buf);

    close(fd);
    return 0;
}