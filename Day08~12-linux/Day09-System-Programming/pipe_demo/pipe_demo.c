#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>

int main(void)
{
    int fd[2] ={0}; // 文件描述符数组：fd[0] 用于读，fd[1] 用于写
  

    char write_msg[] = "哈喽,我是无名管道!";
    char read_msg[100] = {0}; 

    if(pipe(fd) == -1)
    {
        perror("管道创建失败\n");
        return 1;
    }
    pid_t pid = fork();//进程创建必须在管道的后面，因为fork复制父进程的所有资源
    if(pid>0)
    {
        //父进程
        close(fd[0]);//父进程只写，不读，所以关掉
        printf("父进程正在发送数据...\n");
        write(fd[1],write_msg,strlen(write_msg)+1);//写入
        wait(NULL);
        close(fd[1]);//写完关掉,否则read阻塞，不会返回0
      
    }
    else if(pid==0)
    {
        //子进程
        close(fd[1]);
        printf("子进程可以接收到数据吗？ %s\n", read_msg);
        ssize_t ret = read(fd[0],read_msg,sizeof(read_msg));
        printf("read 实际读到的字节数: %ld\n", ret);
        printf("子进程接收到的数据: %s\n", read_msg);
        close(fd[0]);//读完关掉
    }else{

        printf("子进程创建失败...\n");
        return 1;
    }

    return 0;
}