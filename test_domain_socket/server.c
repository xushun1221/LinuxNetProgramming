/*
@Filename : server.c
@Description : Unix Domain Socket IPC test
@Datatime : 2022/06/18 16:29:45
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
#include <ctype.h>

#define SERV_ADDR "server.socket"

int main(int argc, char** argv) {
    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, SERV_ADDR);
    int len = offsetof(struct sockaddr_un, sun_path) + strlen(serv_addr.sun_path);
    unlink(SERV_ADDR);
    bind(listen_fd, (struct sockaddr*)&serv_addr, len);
    listen(listen_fd, 20);

    struct sockaddr_un client_addr;
    int client_addr_len = sizeof(client_addr);
    int client_fd;

    char buf[4096];
    int readbytes;
    while (1) {
        client_addr_len = sizeof(client_addr);
        client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
printf("ok %d\n", client_fd);;
        while ((readbytes = read(client_fd, buf, sizeof(buf))) > 0) {
            for (int i = 0; i < readbytes; ++ i) {
                buf[i] = toupper(buf[i]);
            }
            write(client_fd, buf, readbytes);
        }
        close(client_fd);
    }
    close(listen_fd);
    return 0;
}