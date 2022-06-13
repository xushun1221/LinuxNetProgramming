/*
@Filename : server.c
@Description : test normal epoll server
@Datatime : 2022/06/13 10:49:32
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/epoll.h>
#include "wrap.h"

#define SERV_PORT 8888
#define MAX_BUFSIZE 1024
#define MAX_CLIENTS 1024

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
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;
    char client_ip[INET_ADDRSTRLEN];
    // read buffer
    char buf[MAX_BUFSIZE];
    int read_bytes;

    // create a RBTree for epoll
    int epfd = epoll_create(128);
    if (epfd == -1) {
        perr_exit("epoll_create error");
    }

    // epoll_event
    struct epoll_event clients_events[MAX_CLIENTS]; // returned events set from epoll_wait
    struct epoll_event ep_fdevt; // set info for new fd
    // add listen_fd to RBTree
    ep_fdevt.events = EPOLLIN;
    ep_fdevt.data.fd = listen_fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ep_fdevt);
    if (ret == -1) {
        perr_exit("epoll_ctl error");
    }

    int ret_nready;
    while (1) { // start listen
        ret_nready = epoll_wait(epfd, clients_events, MAX_CLIENTS, -1); // epoll blocking
        if (ret_nready < 0) {
            perr_exit("epoll error");
        } else if (ret_nready > 0) {
            for (int i = 0; i < ret_nready; ++ i) { // clients_events has ret_nready elems
                if (clients_events[i].data.fd == listen_fd) { // new client
                    client_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                    printf("client connected -- ip : %s port : %d\n", 
                        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                        ntohs(client_addr.sin_port)
                    );
                    // add new client to RBTree
                    ep_fdevt.data.fd = client_fd;
                    ep_fdevt.events = EPOLLIN;
                    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ep_fdevt);
                    if (ret == -1) {
                        perr_exit("epoll_ctl error");
                    }
                } else { // client IO
                    read_bytes = Read(clients_events[i].data.fd, buf, sizeof(buf));
                    if (read_bytes == 0) { // client closed
                        ret = epoll_ctl(epfd, EPOLL_CTL_DEL, clients_events[i].data.fd, NULL);
                        if (ret == -1) {
                            perr_exit("epoll_ctl error");
                        }
                        Close(clients_events[i].data.fd); // have to after epoll_ctl
                        printf("client closed\n");
                    } else if (read_bytes > 0) { // read data
                        Write(STDOUT_FILENO, buf, read_bytes);
                        for (int j = 0; j < read_bytes; ++ j) {
                            buf[j] = toupper(buf[j]);
                        }
                        Write(clients_events[i].data.fd, buf, read_bytes);
                    } else {
                        // other error
                    }
                }
            }
        }
    }
    Close(listen_fd);
    return 0;
}