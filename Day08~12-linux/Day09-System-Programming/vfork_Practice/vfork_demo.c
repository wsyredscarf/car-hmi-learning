#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main (void)
{
    pid_t pid = vfork();

    if(pid>0){
        wait(NULL);//等待任何一个子进程结束，并回收它的资源，防止产生僵尸进程
        //父进程后运行，vfork后父进程会被挂起，等子进程执行完并退出后，这里才继续跑
        printf("父进程pid号= %d ,子进程pid号= %d\n",getpid(),pid);
    }
    else if(pid==0)
    {
        //vfork创建的，子进程先运行，父进程后运行。注意点：子进程共享父进程资源，不可直接冲刷缓冲区否则影响父进程。
        printf("子进程pid= %d,父进程pid= %d\n",getpid(),getppid());
        _exit(0);
    }
    else{
        printf("创建失败！！！\n");
    }

    return 0;
}