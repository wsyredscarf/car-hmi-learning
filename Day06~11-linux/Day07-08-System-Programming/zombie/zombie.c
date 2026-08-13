#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int main(void)
{
    
    //fork父子进程运行顺序不定，父进程睡眠。子进程先结束，此时父进程没有收尸子进程，所以子进程是僵尸进程，等30s结束，父进程运行，收尸子进程。
    //在程序运行起来的 30 秒内，立刻打开一个新的终端，输入 ps -ef | grep defunct。会看到名字里带 <defunct> 的进程僵尸
        pid_t pid = fork();
        if(pid > 0)
        {
            //我是父进程，我先睡一会
            printf("我是父进程先睡会,父进程pid= %d,子进程pid= %d\n",getpid(),pid);
            sleep(30);
            printf("父进程睡醒了,程序结束\n");
        }else if(pid == 0)
        {
            //我是子进程
            printf("子进程pid= %d,子进程结束\n",getpid());
        }

    return 0;
}