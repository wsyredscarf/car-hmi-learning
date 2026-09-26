#include "ring_buffer.h"
#include <string.h>

void rb_init(RingBuffer *rb) {
    rb->head = 0;
    rb->tail = 0;
    pthread_mutex_init(&rb->lock, NULL);
}

int rb_write(RingBuffer *rb, const char *data, int len) {
    pthread_mutex_lock(&rb->lock);
    int written = 0;
    for (int i = 0; i < len; i++) {
        int next = (rb->head + 1) % RB_SIZE;
        if (next == rb->tail) break;
        rb->buf[rb->head] = data[i];
        rb->head = next;
        written++;
    }
    pthread_mutex_unlock(&rb->lock);
    return written;
}

int rb_read(RingBuffer *rb, char *data, int len) {
    pthread_mutex_lock(&rb->lock);
    int read_len = 0;
    for (int i = 0; i < len; i++) {
        if (rb->tail == rb->head) break;
        data[i] = rb->buf[rb->tail];
        rb->tail = (rb->tail + 1) % RB_SIZE;
        read_len++;
    }
    pthread_mutex_unlock(&rb->lock);
    return read_len;
}

int rb_empty(RingBuffer *rb) {
    return rb->head == rb->tail;
}