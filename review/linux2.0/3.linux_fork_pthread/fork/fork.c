#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int vrgc,char *vrgv[])
{

/*
fork两处关键点：
    子进程里加 exit(0)，明确告诉它“我结束了”。
    父进程里加 waitpid(pid, NULL, 0)，阻塞等这个子进程退出并回收它。如果没有waitpid函数，则父子进程随机运行
    如果不需要退出码，第二个参数写 NULL 就行；如果需要，就传 &status 再用 WIFEXITED / WEXITSTATUS 解析。
*/
    pid_t pid = fork();//fork创建的进程。父子进程运行先后顺序不确定，需要做防范（如父进程内部用wait函数等待子进程结束回收，防止僵尸进城，子进程内用exit函数结束子进程，告知父进程）
    if(pid<0)
    {
        perror("fork pid fail!");
    }else if(pid == 0)//子进程返回0
    {
        printf("pid=0  parent pid=%d, child pid=%d\n",getppid(),getpid());
        exit(0);/* 子进程退出 */
    }
    else if(pid>0)//父进程返回子进程id
    {
        printf("pid>0   parent pid=%d,child pid=%d\n",getpid(),pid);
        waitpid(pid,NULL,0);/* 回收子进程，防止僵尸 */
    }
    return 0;

//-----------------------------------------------------------------------------------------------------


/*
vfork 的特点
    子进程先运行，父进程挂起（阻塞），直到子进程调用 exec 或 _exit系统调用。不能用 return 或 exit()C库函数调用，否则会清理缓冲区，破坏父进程的栈，导致父进程崩溃或行为异常。
    子进程和父进程共享地址空间（包括栈、数据段），子进程对变量的修改会影响父进程。
    设计初衷是为了让子进程立刻 exec 执行新程序，避免 fork 的页表复制开销。现代 fork 有写时复制（COW），vfork 已很少使用，且不推荐。
*/
    pid_t pid =work();
    if (pid<0)
    {
        perror("vfork fail!");
    }
    else if(pid == 0)/* 子进程：先运行，父进程挂起 */
    {   
        printf("parent pid=%d, child pid=%d\n",getppid(),getpid());
        _exit(0); /* 必须用 _exit，不能用 return */
    }
    else if(pid>0) /* 父进程：等子进程 _exit 后才继续 */
    {
        printf("parent pid=%d, child pid=%d\n",gitpid(),pid);

    }
//--------------------------------------------------------------------------------------------------------------
/*
退出码操作：
  在实际工程中，可以利用 _exit 传状态码让父进程感知子进程的执行结果。
    比如子进程负责读取配置文件，如果文件不存在就 _exit(1)，格式错误就 _exit(2)，成功就 _exit(0)。
    父进程 waitpid 拿到状态后，先通过WIFEXITED判断是否正常退出，然后用WEXITSTATUS拿到具体的错误码，通过switch分支做不同的容错处理，比如使用默认配置、告警或者干脆重启子系统。”



/* 定义子进程的退出码，避免使用魔法数字 */
#define EXIT_CODE_SUCCESS  0   /* 任务成功 */
#define EXIT_CODE_NO_FILE  1   /* 找不到配置文件 */
#define EXIT_CODE_BAD_CFG  2   /* 配置文件格式错误 */
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    } 
    else if (pid == 0) 
    {
        /* ================= 子进程逻辑 ================= */
        printf("[Child] pid=%d, 开始执行任务...\n", getpid());

        /* 模拟业务逻辑：这里我们用 1 代表找不到文件 */
        int task_result = EXIT_CODE_NO_FILE; 

        if (task_result == EXIT_CODE_SUCCESS) {
            printf("[Child] 任务成功完成。\n");
            _exit(EXIT_CODE_SUCCESS);
        } 
        else if (task_result == EXIT_CODE_NO_FILE) {
            printf("[Child] 错误：找不到配置文件！\n");
            _exit(EXIT_CODE_NO_FILE);  /* 传递给父进程的状态码是 1 */
        } 
        else {
            printf("[Child] 错误：配置文件格式错误！\n");
            _exit(EXIT_CODE_BAD_CFG);  /* 传递给父进程的状态码是 2 */
        }
    } 
    else 
    {
        /* ================= 父进程逻辑 ================= */
        int status = 0;
        printf("[Parent] 等待子进程执行结束...\n");

        /* 阻塞等待指定子进程结束，并获取状态 */
        pid_t ret = waitpid(pid, &status, 0);
        if (ret < 0) {
            perror("waitpid");
            return 1;
        }

        /* 关键点：先判断是否是正常退出 */
        if (WIFEXITED(status)) 
        {
            /* 提取子进程的退出码 */
            int exit_code = WEXITSTATUS(status);
            printf("[Parent] 子进程正常退出，退出码：%d\n", exit_code);

            /* 根据子进程的退出码，执行不同的父进程逻辑 */
            switch (exit_code) 
            {
                case EXIT_CODE_SUCCESS:
                    printf("[Parent] 处理：子进程任务成功，继续后续业务。\n");
                    break;

                case EXIT_CODE_NO_FILE:
                    printf("[Parent] 处理：缺少配置文件，采用默认配置启动。\n");
                    break;

                case EXIT_CODE_BAD_CFG:
                    printf("[Parent] 处理：配置文件损坏，发送告警并退出主程序。\n");
                    return 1;  /* 父进程决定退出 */

                default:
                    printf("[Parent] 处理：未知退出码，记录日志。\n");
                    break;
            }
        } 
        else if (WIFSIGNALED(status)) //WIFSIGNALED(status) 返回真，说明子进程是被信号杀死的（比如段错误 SIGSEGV、被 kill -9 杀死、除零 SIGFPE 等）。
        {
            /* 子进程被信号杀死（真正的异常退出） */
            int sig = WTERMSIG(status);//接着调用 WTERMSIG(status)，就能拿到具体的信号编号（比如 11 代表段错误，9 代表强制杀死）
            printf("[Parent] 子进程异常退出，被信号 %d 杀死。\n", sig);
        } 
        else 
        {
            printf("[Parent] 子进程因其他原因退出。\n");
        }
    }

}

/*
总结：
exit  是 C 库函数，会刷新 stdio 缓冲区并调用 atexit 注册的函数；
_exit 是系统调用，直接陷入内核终止进程，不做用户态清理。
在 vfork 的子进程中必须用 _exit，因为父子共享地址空间，用 exit 会刷新共享缓冲区、执行 atexit，破坏父进程状态。C99 引入了标准等价的 _Exit。”

## fork 之后，父子进程在逻辑上有各自独立的地址空间，包括栈、数据段、堆，以及 stdio 缓冲区。所以 fork 的子进程用 exit(0) 是安全的，不会影响父进程。
    但这里有一个关键优化：写时复制（Copy-On-Write,  COW）。
    fork 的写时复制机制：
        fork 时，内核并不立刻复制父进程的全部物理内存。
        父子进程的页表指向同一批物理页，这些页被标记为只读
        当父子任一方尝试写某个页时，触发缺页异常，内核才复制该页，让写方拥有自己的副本。
        因此，初始阶段父子进程物理内存是共享的，但逻辑上地址空间是独立的。任何一方修改数据，都不会影响另一方。
    栈和stdio 缓冲区
    栈：    
        fork 后父子各有独立的栈空间（COW 保护），子进程函数调用、局部变量互不影响。
    stdio缓冲区：
        位于用户空间（通常是堆或静态区），属于进程地址空间的一部分。fork 后子进程有自己的一份副本（COW），子进程 exit 刷新的是自己的缓冲区，父进程的缓冲区不受影响。

## vfork之后，不复制地址空间，子进程直接借用父进程的地址空间，父子完全共享栈、数据、堆、stdio 缓冲区。所以：
    子进程不能修改数据（会直接影响父进程）。
    子进程不能 return（会破坏父进程栈）。
    子进程不能用 exit（会刷新共享的 stdio 缓冲区，执行 atexit，破坏父进程状态）。
    必须用 _exit 或 exec。
## 退出码操作
    在实际工程中，可以利用 _exit 传状态码让父进程感知子进程的执行结果。
    比如子进程负责读取配置文件，如果文件不存在就 _exit(1)，格式错误就 _exit(2)，成功就 _exit(0)。
    父进程 waitpid 拿到状态后，先通过WIFEXITED判断是否正常退出，然后用WEXITSTATUS拿到具体的错误码，通过switch分支做不同的容错处理，比如使用默认配置、告警或者干脆重启子系统。”



*/