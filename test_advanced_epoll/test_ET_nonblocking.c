/*
@Filename : test_ET_nonblocking.c
@Description : 
@Datatime : 2022/06/14 10:07:57
@Author : xushun
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <ctype.h>
#include <fcntl.h>
#include "wrap.h"

#define SERV_PORT 8888
#define MAX_BUFSIZE 10
#define MAX_CLIENTS 10

int main(int argc, char** argv) {
    // listen socket
    int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    Listen(listen_fd, 128);
    // client socket
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr_len);
    int client_fd;
    char client_ip[INET_ADDRSTRLEN];
    // read buffer
    char buf[MAX_BUFSIZE];
    int read_bytes;

    struct epoll_event event;
    struct epoll_event events[MAX_CLIENTS];
    int epfd = epoll_create(10);
    // just one client for test
    client_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    printf("client connected\n");
    // ------------------------- // non-blocking
    int flag = fcntl(listen_fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(listen_fd, F_SETFL, flag);
    // -------------------------
    event.data.fd = client_fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event);

    int ret_nready;
    while (1) {
        ret_nready = epoll_wait(epfd, events, MAX_CLIENTS, -1);
        if (events[0].data.fd == client_fd) {
            // ---------------------------
            while ((read_bytes = Readn(client_fd, buf, MAX_BUFSIZE / 2)) > 0) {
                Write(STDOUT_FILENO, buf, read_bytes);
            }
            // ---------------------------
        }
    }
    Close(listen_fd);
    return 0;
}