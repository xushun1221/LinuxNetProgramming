/*
@Filename : server.c
@Description : test tcp socket, server, upper chars from client
@Datatime : 2022/06/09 10:30:10
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

#define SERV_PORT 8888

int main(int argc, char** argv) {
    int ret;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0); // sys/scoket.h
    if (listen_fd == -1) {
        perror("socket error"); exit(1);
    }
    struct sockaddr_in serv_addr; // arpa/inet.h
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT); // arpa/inet.h
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(listen_fd, (struct scokaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == -1) {
        perror("bind error"); exit(1);
    }
    ret = listen(listen_fd, 128);
    if (ret == -1) {
        perror("listen error"); exit(1);
    }
    struct sockaddr_in clit_addr;
    socklen_t clit_addr_len = sizeof(clit_addr);
    int clit_fd = accept(listen_fd, (struct sockaddr*)&clit_addr, &clit_addr_len);
    if (clit_fd == -1) {
        perror("accept error"); exit(1);
    }
    char clit_IP[1024];
    printf("client ip : %s port : %d\n", 
        inet_ntop(AF_INET, &clit_addr.sin_addr.s_addr, clit_IP, sizeof(clit_IP)),
        ntohs(clit_addr.sin_port)
    );
    // char read_buf[BUFSIZ]; // BUFSIZ stdio.h 中定义的默认缓存区大小 8192
    char buf[4096];
    int read_bytes;
    while (1) {
        read_bytes = read(clit_fd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, read_bytes);
        for (int i = 0; i < read_bytes; ++ i) {
            buf[i] = toupper(buf[i]); // ctype.h
        }
        write(clit_fd, buf, read_bytes);
    }
    close(listen_fd);
    close(clit_fd);
    return 0;
}