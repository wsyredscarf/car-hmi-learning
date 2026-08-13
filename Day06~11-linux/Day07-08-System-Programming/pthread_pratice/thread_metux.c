#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>


/*
补充说明：
    操作系统进行“线程切换（上下文切换）”是需要时间的（大概在微秒级）。这就是为啥机器能运行，刚开始量小怎么测都没问题，
    但是运行成千上万次，出现问题的几率会越来越大
    不加锁：
        for的循环次数越少，单线程在1微秒不到就直接能完成了，完整的切换上下文（线程切换）就没有数据问题。
        如果for的次数很大，达到上千次，上万次，出错的几率越大。还有一种是执行文件越快，出错的机率越大
    加锁：
        加上锁后，当t1线程执行变量++时，因为带锁，所以t2无法拿到变量，无法操作。所以每次操作都是安全的，没有其他线程改变变量大小
锁的作用就是强迫线程 A 必须“读完->算完->写完”这一整串动作做完，才能放线程 B 进来读，从而保证绝对的原子性。
操作系统会把 CPU 使用权在 A 线程和 B 线程之间来回切分。
致命的底层原因（非原子操作）： 在 CPU 的底层汇编指令里，global_count++ 并不是一步完成的，它分为三步：
    读取（Load）：把内存里的 global_count 读到 CPU 寄存器里。
    计算（Add）：在寄存器里加 1。
    写入（Store）：把新值写回内存。
*/


int global_count = 0; // 全局变量，所有线程共享
pthread_mutex_t mutex; // 定义一个互斥锁变量

//无锁版本，全局变量自增
void *fun_without_lock(void *value)
{
    for(int i=0;i<100000;i++)
    {
        global_count++;
    }

    return NULL;
}

//有锁版本，全局变量自增
void *fun_with_lock(void *value)
{
    for(int i=0;i<100000;i++)
    {
        //操作变量之前增加锁，进行保护
        pthread_mutex_lock(&mutex);
        
        global_count++;
        
        //变量操作完后，立刻释放锁，否则其他线程会锁死
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(void)
{
    //创建两个线程变量
    pthread_t t1 ,t2;

    //创建锁
    // pthread_mutex_init 参数：
    // 参数1：&mutex (锁的地址)
    // 参数2：NULL (锁的属性，默认普通锁)
    pthread_mutex_init(&mutex,NULL);

    // //创建两个线程,不带锁
    // pthread_create(&t1,NULL,fun_without_lock,&global_count);//不带锁
    // pthread_create(&t2,NULL,fun_without_lock,&global_count);//不带锁
    // pthread_join(t1,NULL);//等待子线程tid
    // pthread_join(t2,NULL);//等待子线程tid
    // printf("【无锁】最终结果：%d (正常应该是 200000,无锁会导致数据丢失)\n", global_count);//数据会乱，global_count会变成随机数

    // //-----
    //创建两个线程，带锁
    pthread_create(&t1,NULL,fun_with_lock,&global_count);
    pthread_create(&t2,NULL,fun_with_lock,&global_count);
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    printf("【带锁】最终结果：%d (正常应该是 200000,互斥锁保护了数据)\n", global_count);

    pthread_mutex_destroy(&mutex);//销毁锁，释放系统资源

    int global_count = 0;
    return 0;

}