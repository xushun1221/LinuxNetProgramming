/*
@Filename : server.c
@Description : udp server test
@Datatime : 2022/06/18 14:03:53
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>

#define SERV_PORT 8888
#define MAX_BUFSIZE 4096

int main(int argc, char** argv) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    char buf[MAX_BUFSIZE];
    int recvbytes, sendbytes;
    while (1) {
        recvbytes = recvfrom(listen_fd, buf, MAX_BUFSIZE, 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (recvbytes == -1) {
            perror("recvfrom error");
            exit(-1);
        }
        write(STDOUT_FILENO, buf, recvbytes);
        for (int i = 0; i < recvbytes; ++ i) {
            buf[i] = toupper(buf[i]);
        }
        sendbytes = sendto(listen_fd, buf, recvbytes, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        if (sendbytes == -1) {
            perror("sendto error");
            exit(-1);
        }
    }
    close(listen_fd);
    return 0;
}