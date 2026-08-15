#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 9999

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    printf("UDP服务端监听 %d\n", PORT);

    char buf[128] = {0};
    struct sockaddr_in cli_addr;
    socklen_t len = sizeof(cli_addr);
    recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cli_addr, &len);
    printf("UDP接收:%s\n", buf);
    sendto(fd, "udp reply", sizeof("udp reply"), 0, (struct sockaddr *)&cli_addr, len);
    close(fd);
    return 0;
}