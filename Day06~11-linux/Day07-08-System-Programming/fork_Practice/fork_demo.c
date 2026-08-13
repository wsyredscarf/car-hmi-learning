#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


int main(void)
{
    pid_t pid = fork();//开始分裂（可以理解为，父进程就是使用fork之前的状态，使用后父进程分裂创建子进程 并生成pid号）
    //误区：getpid()函数是求当前状态下的pid号，在父进程里是求的父进程pid，子进程里求的是子进程pid
    //getppid()求的是当前进程的父进程

    //父进程拥有子进程的pid号，创建成功后返回子进程pid。  
    if(pid > 0){

        //父进程代码
        printf("父进程pid =%d, 子进程pid =%d\n",getpid(),pid);
        sleep(2);//可以替换为wait
    }
    //子进程则返回0
    else if(pid==0){
        //子进程代码
        printf("子进程pid =%d,父进程pid =%d\n",getpid(),getppid());
    }
    else{
        perror("fork fail\n");
        return -1;
    }

    return 0;
}