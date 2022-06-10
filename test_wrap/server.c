/*
@Filename : server.c
@Description : 
@Datatime : 2022/06/10 15:38:51
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
#include <ctype.h>
#include "wrap.h"

#define SERV_PORT 8888

int main(int argc, char** argv) {
    int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listen_fd, (struct scokaddr*)&serv_addr, sizeof(serv_addr));
    Listen(listen_fd, 128);
    struct sockaddr_in clit_addr;
    socklen_t clit_addr_len = sizeof(clit_addr);
    int clit_fd = Accept(listen_fd, (struct sockaddr*)&clit_addr, &clit_addr_len);
    char clit_IP[1024];
    printf("client ip : %s port : %d\n", 
        inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, clit_IP, sizeof(clit_IP)),
        ntohs(clit_addr.sin_port)
    );
    // char read_buf[BUFSIZ]; // BUFSIZ stdio.h 中定义的默认缓存区大小 8192
    char buf[4096];
    int read_bytes;
    while (1) {
        read_bytes = Read(clit_fd, buf, sizeof(buf));
        Write(STDOUT_FILENO, buf, read_bytes);
        for (int i = 0; i < read_bytes; ++ i) {
            buf[i] = toupper(buf[i]); // ctype.h
        }
        Write(clit_fd, buf, read_bytes);
    }
    Close(listen_fd);
    Close(clit_fd);
    return 0;
}