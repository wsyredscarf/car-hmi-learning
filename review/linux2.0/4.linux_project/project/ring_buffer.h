#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <pthread.h>

#define RB_SIZE 4096

typedef struct {
    char buf[RB_SIZE];
    int head;
    int tail;
    pthread_mutex_t lock;
} RingBuffer;

/* 函数声明：只写原型，不写函数体，后面加分号 */
void rb_init(RingBuffer *rb);
int rb_write(RingBuffer *rb, const char *data, int len);
int rb_read(RingBuffer *rb, char *data, int len);
int rb_empty(RingBuffer *rb);

#endif