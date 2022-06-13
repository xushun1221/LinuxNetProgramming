/*
@Filename : test_ET.c
@Description : test ET epoll
@Datatime : 2022/06/13 15:52:36
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "wrap.h"

#define MAX_BUFSIZE 10

int main(int argc, char** argv) {
    int pipe_fd[2];
    pipe(pipe_fd);
    char buf[MAX_BUFSIZE];
    pid_t pid = fork();
    if (pid == 0) { // child
        Close(pipe_fd[0]); // close read end
        char ch = 'a';
        while (1) { // loop  write 10 chars to parent per 5 secs
            int i;
            for (i = 0; i < MAX_BUFSIZE / 2; ++ i) {
                buf[i] = ch;
            }
            buf[i - 1] = '\n';
            ++ ch;
            for (; i < MAX_BUFSIZE; ++ i) {
                buf[i] = ch;
            }
            buf[i - 1] = '\n';
            ++ ch;
            Write(pipe_fd[1], buf, MAX_BUFSIZE);
            sleep(5);
        }
        Close(pipe_fd[1]);
    } else if (pid > 0) { // parent
        Close(pipe_fd[1]); // close write end
        struct epoll_event event;
        struct epoll_event events[10];
        int epfd = epoll_create(10);
        event.data.fd = pipe_fd[0]; // add read end to RBTree
        // LT
        // event.events = EPOLLIN;
        // ET
        event.events = EPOLLIN | EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_ADD, pipe_fd[0], &event);

        while (1) { // read 5 chars from child
            int ret_nready = epoll_wait(epfd, events, 10, -1);
            if (events[0].data.fd == pipe_fd[0]) {
                int read_bytes =  Read(pipe_fd[0], buf, MAX_BUFSIZE / 2);
                Write(STDOUT_FILENO, buf, read_bytes);
            }
        }
        Close(pipe_fd[0]);
        Close(epfd);
    }
    return 0;
}