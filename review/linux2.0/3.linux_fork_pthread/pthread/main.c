


/*
1.线程的基本使用方法

    线程工作函数：打印自己的信息 
    static void *thread_worker(void *arg)
    {
        (void)arg;  //消除未使用参数警告 
        printf("[thread] pid=%d, tid=%lu\n", getpid(), (unsigned long)pthread_self());
        return NULL;
    }

    int thread_basic_demo(void)
    {
        pthread_t tid;

        //关键点1：pthread_create创建线程，成功返回0，失败返回错误码 
        int ret = pthread_create(&tid, NULL, thread_worker, NULL);
        if (ret != 0) {
            fprintf(stderr, "pthread_create: %d\n", ret);
            return -1;
        }

        printf("[main] before join\n");

        //关键点2：pthread_join阻塞等待线程结束，类似进程的waitpid ，回收子线程
        pthread_join(tid, NULL);

        printf("[main] after join\n");
        return 0;
    }
关键点解析：
    pthread_create 的返回值不能用 perror，它不是系统调用，返回的是错误码（如 EAGAIN、EINVAL），要用 strerror(ret)。
    pthread_join 让主线程阻塞，直到指定线程退出。如果不调用，主线程可能先退出，导致整个进程结束，子线程被强行终止。
    pthread_self() 返回当前线程ID，类型是 pthread_t，通常用 %lu 打印。

*/
//------------------------------------------------------------------------------------------------

/*
2.互斥锁
    #define LOOP_COUNT 100000

    static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
    static int g_count = 0;  //共享变量，需要加锁保护

    static void *thread_worker(void *arg)
    {
        (void)arg;
        for (int i = 0; i < LOOP_COUNT; i++) {
            //关键点1：临界区加锁，保证自增原子性 
            pthread_mutex_lock(&g_mutex);
            g_count++;
            pthread_mutex_unlock(&g_mutex);
        }
        return NULL;
    }

    int mutex_demo(void)
    {
        pthread_t tid1, tid2;

        pthread_create(&tid1, NULL, thread_worker, NULL);
        pthread_create(&tid2, NULL, thread_worker, NULL);

        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);

        printf("g_count = %d (expect %d)\n", g_count, LOOP_COUNT * 2);

        //关键点2：销毁锁
        pthread_mutex_destroy(&g_mutex);
        return 0;
    }

关键点解析：
    两个线程同时对 g_count 做 ++，不加锁会产生竞态，结果可能小于 200000。
    pthread_mutex_lock 在锁已被占用时阻塞，直到其他线程 unlock。
    加锁的代码区间叫临界区，应尽量短，只包含必须保护的共享数据操作。
    锁用完必须 destroy，否则资源泄漏。
*/
//------------------------------------------------------------------------------------------------

/*
3.条件变量与生产者-消费者

    #define BUFFER_SIZE 10
    #define PRODUCE_COUNT 20

    static int g_buffer[BUFFER_SIZE];
    static int g_count = 0;
    static int g_in = 0;
    static int g_out = 0;

    static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;//互斥锁变量
    static pthread_cond_t g_not_full = PTHREAD_COND_INITIALIZER;//条件变量
    static pthread_cond_t g_not_empty = PTHREAD_COND_INITIALIZER;//条件变量

    static void *producer(void *arg)
    {
        (void)arg;
        for (int i = 0; i < PRODUCE_COUNT; i++) {
            pthread_mutex_lock(&g_mutex); //加锁

            //关键点1：必须用while判断，防止虚假唤醒 
            while (g_count == BUFFER_SIZE) {
                pthread_cond_wait(&g_not_full, &g_mutex);
            }

            g_buffer[g_in] = i;
            g_in = (g_in + 1) % BUFFER_SIZE;
            g_count++;
            printf("[producer] produce %d, count=%d\n", i, g_count);

            //关键点2：通知消费者
            pthread_cond_signal(&g_not_empty);
            pthread_mutex_unlock(&g_mutex);
            usleep(10000);
        }
        return NULL;
    }

    static void *consumer(void *arg)
    {
        (void)arg;
        for (int i = 0; i < PRODUCE_COUNT; i++) {
            pthread_mutex_lock(&g_mutex);

            while (g_count == 0) {
                pthread_cond_wait(&g_not_empty, &g_mutex);
            }

            int val = g_buffer[g_out];
            g_out = (g_out + 1) % BUFFER_SIZE;
            g_count--;
            printf("[consumer] consume %d, count=%d\n", val, g_count);

            pthread_cond_signal(&g_not_full);
            pthread_mutex_unlock(&g_mutex);
            usleep(15000);
        }
        return NULL;
    }

    int pc_demo(void)
    {
        pthread_t prod, cons;
        pthread_create(&prod, NULL, producer, NULL);
        pthread_create(&cons, NULL, consumer, NULL);

        pthread_join(prod, NULL);
        pthread_join(cons, NULL);

        pthread_mutex_destroy(&g_mutex);
        pthread_cond_destroy(&g_not_full);
        pthread_cond_destroy(&g_not_empty);
        return 0;
    }
关键点解析：
    必须用 while 判断条件，不能用 if。pthread_cond_wait 可能被虚假唤醒，醒来后条件仍可能不满足。
    pthread_cond_wait 内部会释放锁并阻塞，被唤醒后重新加锁再返回。
    生产者写完 signal 通知消费者，消费者读完 signal 通知生产者。
    缓冲区用 (in + 1) % SIZE 实现环形复用。
*/
//------------------------------------------------------------------------------------------------


