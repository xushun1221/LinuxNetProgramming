/*
@Filename : client.c
@Description : test tcp socket, client, upper chars from client
@Datatime : 2022/06/09 11:32:06
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERV_PORT 8888

int main(int argc, char** argv) {
    int ret;
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket error"); exit(1);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
    ret = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == -1) {
        perror("connect error"); exit(1);
    }
    int read_bytes;
    char buf[4096];
    while (1) {
        read_bytes = read(STDIN_FILENO, buf, sizeof(buf));
        write(client_fd, buf, read_bytes);
        read_bytes = read(client_fd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, read_bytes);
    }
    close(client_fd);
    return 0;
}