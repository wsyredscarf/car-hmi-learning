#define _DEFAULT_SOURCE
#include "uart.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>


int uart_open(const char *dev, speed_t baud) {
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) { perror("open uart"); return -1; }

    struct termios tio;
    if (tcgetattr(fd, &tio) != 0) { perror("tcgetattr"); close(fd); return -1; }

    cfmakeraw(&tio);
    cfsetispeed(&tio, baud);
    cfsetospeed(&tio, baud);

    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CRTSCTS;

    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 10;

    tcflush(fd, TCIOFLUSH);
    if (tcsetattr(fd, TCSANOW, &tio) != 0) { perror("tcsetattr"); close(fd); return -1; }

    return fd;
}

/* ============ 设置非阻塞 ============ */
int set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}