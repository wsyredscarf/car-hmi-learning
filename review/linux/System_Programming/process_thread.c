/*
维度	      进程 (Process)	                                      线程 (Thread)
定义	        资源分配的基本单位	                                    CPU调度的基本单位
地址空间	    独立，互不干扰	                                        共享同一进程的地址空间
资源开销	    大（创建/切换需内核介入，开销大）	                      小（共享进程资源，切换开销小）
通信方式	    IPC（管道、消息队列、共享内存、信号、Socket）	          全局变量、互斥锁、条件变量
稳定性	        一个进程崩溃不影响其他进程	                             一个线程崩溃会导致整个进程崩溃
适用场景	    多任务隔离、高稳定性、网关主进程	                      并发处理、任务调度、IO密集型
口述：进程是资源分配单位，线程是调度单位。进程有独立地址空间，线程共享进程资源，所以线程更轻量，但稳定性差。

子进程复制进程fork以后的资源
pid_t pid = fork();
if (pid < 0) {
    perror("fork");
} else if (pid == 0) {
    // 子进程
    printf("child pid = %d, parent pid = %d\n", getpid(), getppid());
} else {
    // 父进程
    printf("parent pid = %d, child pid = %d\n", getpid(), pid);
}

------------------------

线程的基本操作：
void *thread_func(void *arg)
 {
    int *num = (int *)arg;
    printf("thread got %d\n", *num);
    return NULL;
}
int main()
{
    pthread_t tid;
    int arg = 10;
    pthread_create(&tid, NULL, thread_func, &arg);
    pthread_join(tid, NULL); // 等待线程结束
    return 0;
}
##
互斥锁（Mutex）：
保护共享资源，防止竞态条件。
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(&mutex);
// 临界区
pthread_mutex_unlock(&mutex);


##
条件变量（Condition Variable）
用于线程同步。必须用 while 判断条件，防止虚假唤醒。

pthread_mutex_lock(&mutex);
while (condition_is_false) {
    pthread_cond_wait(&cond, &mutex); // 释放锁并等待，被唤醒后重新拿锁
}
// 处理
pthread_mutex_unlock(&mutex);

##
信号量（Semaphore）：

sem_t sem;
sem_init(&sem, 0, 1); // 初始值为1
sem_wait(&sem);   // P操作，减1
// 临界区
sem_post(&sem);   // V操作，加1

线程共享哪些资源？独立哪些？
共享：代码段、数据段、堆、文件描述符、信号处理。
独立：栈、寄存器、程序计数器、线程ID。

多进程 vs 多线程怎么选？
多进程：稳定性高、隔离性好，但IPC复杂，适合网关主进程。
多线程：并发高、资源开销小，但调试复杂，适合IO密集型任务。

死锁的条件和避免方法？
条件：互斥、占有且等待、不可抢占、循环等待。
避免：加锁顺序一致、使用 trylock、超时机制。

fork 后父子进程共享什么？
共享文件描述符（文件偏移量）、共享内存（需显式创建）。
不共享：全局变量、堆、栈（写时复制，写时独立）。

*/