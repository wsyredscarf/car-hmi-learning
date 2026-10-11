#ifndef UART_H
#define UART_H

#include <termios.h>  /* speed_t 类型需要这个头文件 */

/* 函数声明 */
int uart_open(const char *dev, speed_t baud);
int set_nonblock(int fd);

#endif