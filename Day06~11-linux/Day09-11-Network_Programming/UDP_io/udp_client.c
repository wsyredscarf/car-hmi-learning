#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 9999
#define IP "127.0.0.1"
int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &srv_addr.sin_addr);

    char msg[] = "hello udp";
    sendto(fd, msg, strlen(msg), 0, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    char recv[128] = {0};
    socklen_t len = sizeof(srv_addr);
    recvfrom(fd, recv, sizeof(recv), 0, (struct sockaddr *)&srv_addr, &len);
    printf("udp服务端回复:%s\n", recv);
    close(fd);
    return 0;
}