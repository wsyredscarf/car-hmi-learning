/*
手写串口数据解析状态机（对应：设备通信、数据采集、状态机）,核心逻辑处理如下

用状态机+环形缓冲区来处理串口粘包，Linux下我会用epoll监听串口fd（/dev/ttyS*），读取数据后喂给状态机解析
// 状态定义
typedef enum { STATE_HEAD, STATE_LEN, STATE_DATA, STATE_CRC } ParseState;

typedef struct {
    uint8_t buf[256];
    int index;
    ParseState state;
    uint8_t data_len;
} UartParser;

void uart_parse_byte(UartParser *p, uint8_t byte) {
    switch (p->state) {
        case STATE_HEAD:
            if (byte == 0xAA) p->state = STATE_LEN; // 假设帧头0xAA
            break;
        case STATE_LEN:
            p->data_len = byte;
            p->index = 0;
            p->state = STATE_DATA;
            break;
        case STATE_DATA:
            p->buf[p->index++] = byte;
            if (p->index >= p->data_len) p->state = STATE_CRC;
            break;
        case STATE_CRC:
            // 校验CRC并提交数据
            p->state = STATE_HEAD;
            break;
    }
}

*/



/*

环形缓冲区（Ring Buffer）:

#define BUF_SIZE 1024
typedef struct {
    char buf[BUF_SIZE];
    volatile int head;  // 写指针
    volatile int tail;  // 读指针
} RingBuffer;

int rb_write(RingBuffer *rb, const char *data, int len) {
    int i;
    for (i = 0; i < len; i++) {
        int next = (rb->head + 1) % BUF_SIZE;
        if (next == rb->tail) break;  // 缓冲区满
        rb->buf[rb->head] = data[i];
        rb->head = next;
    }
    return i;  // 实际写入长度
}

int rb_read(RingBuffer *rb, char *data, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (rb->tail == rb->head) break;  // 缓冲区空
        data[i] = rb->buf[rb->tail];
        rb->tail = (rb->tail + 1) % BUF_SIZE;
    }
    return i;  // 实际读出长度
}

*/