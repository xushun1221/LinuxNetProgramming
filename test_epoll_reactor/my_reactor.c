/*
@Filename : my_reactor.c
@Description : epoll - Reactor server 不怎么重要的错误检查就不写了
@Datatime : 2022/06/15 16:55:08
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>

#define MAX_EVENTS 1000
#define MAX_BUFSIZE 4096
#define SERV_PORT 8888

struct myevent_s {
    int fd;
    int events;
    void* arg;
    void (*call_back)(int fd, int events, void* arg);
    int status;
    char buf[MAX_BUFSIZE];
    int buf_len;
    long last_active;
};

int g_epfd;
struct myevent_s g_events[MAX_EVENTS + 1];

void myeventset(struct myevent_s* ev, int fd, void (*call_back)(int fd, int events, void* arg), void* arg, int keep_buf) {
    ev -> fd = fd;
    ev -> call_back = call_back;
    ev -> events = 0;
    ev -> arg = arg;
    ev -> status = 0;
    ev -> last_active = time(NULL);
    if (keep_buf == 0) {
        memset(ev -> buf, 0, sizeof(ev -> buf));
        ev -> buf_len = 0;
    }
    return;
}

void eventadd(int epfd, int events, struct myevent_s* ev) {
    struct epoll_event epev;
    epev.data.ptr = ev;
    epev.events = events;
    ev -> events = events;
    int op;
    if (ev -> status == 0) {
        ev -> status = 1;
        op = EPOLL_CTL_ADD;
    }
    if (epoll_ctl(epfd, op, ev -> fd, &epev) < 0) {
        printf("event add failed  [fd=%d], [op=%d], [event=%d]\n", ev -> fd, op, events);
    } else {
        printf("event add success [fd=%d], [op=%d], [event=%d]\n", ev -> fd, op, events);
    }
    return;
}

void eventdel(int epfd, struct myevent_s* ev) {
    int op;
    if (ev -> status == 1) {
        ev -> status = 0;
        op = EPOLL_CTL_DEL;    
    }
    if (epoll_ctl(epfd, op, ev -> fd, NULL) < 0) {
        printf("event del failed  [fd=%d], [op=%d]\n", ev -> fd, op);
    } else {
        printf("event del success [fd=%d], [op=%d]\n", ev -> fd, op);
    }
    return;
}

void senddata(int fd, int events, void* arg);
void recvdata(int fd, int events, void* arg) {
    struct myevent_s* ev = (struct myevent_s*)arg;
    int recv_bytes = recv(fd, ev -> buf, sizeof(ev -> buf), 0);
    eventdel(g_epfd, ev);
    if (recv_bytes > 0) {
        ev -> buf_len = recv_bytes;
        ev -> buf[recv_bytes] = '\0';
        printf("recv[fd=%d], [%d]%s\n", fd, recv_bytes, ev -> buf);
        myeventset(ev, fd, senddata, ev, 1);
        eventadd(g_epfd, EPOLLOUT, ev);
    } else if (recv_bytes == 0) {
        printf("[fd=%d], [pos=%d] closed\n", fd, (int)(ev - g_events));
        close(fd);
    } else {
        printf("recv[fd=%d] error %s\n", fd, strerror(errno));
    }
    return;
}

void senddata(int fd, int events, void* arg) {
    struct myevent_s* ev = (struct myevent_s*)arg;
    int send_bytes = send(fd, ev -> buf, ev -> buf_len, 0);
    eventdel(g_epfd, ev);
    if (send_bytes > 0) {
        printf("send[fd=%d], [%d]%s\n", fd, send_bytes, ev -> buf);
        myeventset(ev, fd, recvdata, ev, 0);
        eventadd(g_epfd, EPOLLIN, ev);
    } else {
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
        close(fd);
    }
}

void acceptconn(int listen_fd, int events, void* arg) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr_len);
    int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    int i;
    for (i = 0; i < MAX_EVENTS; ++ i) {
        if (g_events[i].status == 0) {
            break;
        }
    }
    if (i == MAX_EVENTS) {
        printf("cannot accept more clients [fd=%d]\n", client_fd);
        return;
    }
    int flag = fcntl(client_fd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(client_fd, F_SETFL, flag);
    myeventset(&g_events[i], client_fd, recvdata, &g_events[i], 0);
    eventadd(g_epfd, EPOLLIN, &g_events[i]);
    printf("new connect [%s:%d], [time=%ld], [pos=%d]\n",
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), g_events[i].last_active, i
    );
    return;
}

void initlistensocket(int epfd, unsigned short nport) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = nport;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listen_fd, 20);

    myeventset(&g_events[MAX_EVENTS], listen_fd, acceptconn, &g_events[MAX_EVENTS], 0);
    eventadd(epfd, EPOLLIN, &g_events[MAX_EVENTS]);
    return;
}


int main(int argc, char** argv) {
    unsigned short nport = htons(SERV_PORT);
    if (argc == 2) {
        nport = htons(atoi(argv[1]));
    }
    g_epfd = epoll_create(MAX_EVENTS + 1);
    initlistensocket(g_epfd, nport);
    struct epoll_event events[MAX_EVENTS + 1];
    printf("server running : [port=%d]\n", ntohs(nport));

    int checkpos = 0;
    while (1) {
        long now = time(NULL);
        for (int i = 0; i < 100; ++ i, ++ checkpos) {
            if (checkpos == MAX_EVENTS) {
                checkpos = 0;
            }
            if (g_events[checkpos].status != 1) {
                continue;
            }
            long duration = now - g_events[checkpos].last_active;
            if (duration >= 60) {
                eventdel(g_epfd, &g_events[checkpos]);
                close(g_events[checkpos].fd);
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
            }
        }

        int nready = epoll_wait(g_epfd, events, MAX_EVENTS + 1, 1000);
        if (nready < 0) {
            printf("epoll_wait error\n");
            return -1;
        }

        for (int i = 0; i < nready; ++ i) {
            struct myevent_s* ev = (struct myevent_s*)events[i].data.ptr;
            if ((events[i].events & EPOLLIN) && (ev -> events & EPOLLIN)) {
                ev -> call_back(ev -> fd, events[i].events, ev -> arg);
            }
            if ((events[i].events & EPOLLOUT) && (ev -> events & EPOLLOUT)) {
                ev -> call_back(ev -> fd, events[i].events, ev -> arg);
            }
        }
    }
    return 0;
}