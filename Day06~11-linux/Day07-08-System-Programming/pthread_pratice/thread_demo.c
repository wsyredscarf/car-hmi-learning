#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

// 线程执行函数，参数必须固定为 void* 指针类型
// 返回类型也必须固定为 void* 指针
void *thread_fun(void *arg)
{
    int *num = (int*)arg;
    printf("子线程收到参数= %d,正在干活\n",*num);
    sleep(5);// 模拟耗时操作
    printf("子线程干完了！\n");

    return NULL;// 子线程结束

}


int main(void)
{
    pthread_t tid = 0; // pthread_t 是专门用来存储“线程ID”的数据类型
    int num = 888;

    //创建线程
    // pthread_create 参数详解：
    // 参数1：&tid (指向线程 ID 变量的指针，内核会把新线程的 ID 写进去)
    // 参数2：NULL (线程属性，NULL 表示使用默认属性)
    // 参数3：thread_func (新线程要执行的函数名/指针)
    // 参数4：&value (传给线程函数的参数，这里传了 value 的地址)
    if(pthread_create(&tid,NULL,thread_fun,&num) !=0)
    {
        perror("创建线程失败\n");
        return 1;
    }

    printf("主线程还在运行...\n");
    // pthread_join 参数详解：
    // 参数1：tid (要等待的线程 ID)
    // 参数2：NULL (用于接收线程函数的返回值，不需要就不管)
    // 作用：主线程会阻塞在这里，直到 tid 代表的子线程执行完毕才能继续往下走。
    pthread_join(tid,NULL);
    printf("主线程结束！\n");
    return 0;
}