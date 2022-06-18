/*
@Filename : client.c
@Description : Unix domain socket IPC test
@Datatime : 2022/06/18 16:12:24
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stddef.h>

#define CLIT_ADDR "client.socket"
#define SERV_ADDR "server.socket"

int main(int argc, char** argv) {
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, CLIT_ADDR);
    int len = offsetof(struct sockaddr_un, sun_path) + strlen(client_addr.sun_path);
    unlink(CLIT_ADDR);
    bind(client_fd, (struct sockaddr*)&client_addr, len);

    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SERV_ADDR);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(serv_addr.sun_path);
    connect(client_fd, (struct sockaddr*)&serv_addr, len);

    char buf[4096];
    int readbytes;
    while ((readbytes = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        write(client_fd, buf, readbytes);
        readbytes = read(client_fd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, readbytes);
    }
    close(client_fd);
    return 0;
}