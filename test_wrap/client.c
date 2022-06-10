/*
@Filename : client.c
@Description : 
@Datatime : 2022/06/10 15:38:47
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
#include "wrap.h"

#define SERV_PORT 8888

int main(int argc, char** argv) {
    int client_fd = Socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
    Connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    int read_bytes;
    char buf[4096];
    while (1) {
        read_bytes = Read(STDIN_FILENO, buf, sizeof(buf));
        Write(client_fd, buf, read_bytes);
        read_bytes = Read(client_fd, buf, sizeof(buf));
        Write(STDOUT_FILENO, buf, read_bytes);
    }
    close(client_fd);
    return 0;
}