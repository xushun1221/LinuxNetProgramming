/*
@Filename : client.c
@Description : udp client test
@Datatime : 2022/06/18 13:42:37
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

#define SERV_PORT 8888
#define SERV_ADDR "127.0.0.1"
#define MAX_BUFSIZE 4096

int main(int argc, char** argv) {
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_ADDR, &serv_addr.sin_addr.s_addr);

    int client_fd = socket(AF_INET, SOCK_DGRAM, 0);

    char buf[MAX_BUFSIZE];
    int readbytes, sendbytes, recvbytes;
    while ((readbytes = read(STDIN_FILENO, buf, MAX_BUFSIZE)) > 0) {
        sendbytes = sendto(client_fd, buf, readbytes, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (sendbytes == -1) {
            perror("sendto error");
            exit(-1);
        }
        recvbytes = recvfrom(client_fd, buf, MAX_BUFSIZE, 0, NULL, 0); // NULL 不关心对端地址
        if (recvbytes == -1) {
            perror("recvfrom error");
            exit(-1);
        }
        write(STDOUT_FILENO, buf, recvbytes);
    }
    close(client_fd);
    return 0;
}