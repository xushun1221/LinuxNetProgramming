/*
@Filename : client.c
@Description : libevent TCP client
@Datatime : 2022/06/20 21:12:43
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#define SERV_PORT 8888
#define SERV_ADDR "127.0.0.1"
#define MAX_BUFSIZE 1024

void read_cb(struct bufferevent* bev, void* ctx) {
    char buf[MAX_BUFSIZE] = {0};
    bufferevent_read(bev, buf, sizeof(buf));
    // 从终端写的数据发送给server
    // 从server读取的数据显示到终端
    if (buf[0] == '#') {
        bufferevent_write(bev, buf + 1, strlen(buf));
    } else {
        printf("%s\n", buf);
    }
    return;
}

void write_cb(struct bufferevent* bev, void* ctx) {
    printf("client : send to server success, callback called\n");
    return;
}

void event_cb(struct bufferevent* bev, short events, void* ctx) {
    if (events & BEV_EVENT_EOF) {
        printf("client closed\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("bufferevent error\n");
    } else if (events & BEV_EVENT_CONNECTED){
        printf("connected to server\n");
        return;
    } else {
        return;
    }
    bufferevent_free(bev);
    printf("bufferevent free\n");
    return;
}

// read bytes from terminal
void read_terminal(evutil_socket_t fd, short what, void* arg) {
    char buf[MAX_BUFSIZE] = {0};
    int read_bytes = read(fd, buf + 1, sizeof(buf) - 1);
    buf[0] = '#';
    struct bufferevent* bev = (struct bufferevent*)arg;
    bufferevent_write(bev, buf, read_bytes + 2);
}

int main(int argc, char** argv) {
    // base
    struct event_base* base = event_base_new();
    // client fd -> bufferevent
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct bufferevent* bev = bufferevent_socket_new(base, client_fd, BEV_OPT_CLOSE_ON_FREE);
    // server info
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_ADDR, &serv_addr.sin_addr.s_addr);
    // connect to server
    bufferevent_socket_connect(bev, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    // set callback
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
    // enable read
    bufferevent_enable(bev, EV_READ);

    // event : listen bytes from terminal
    struct event* ev = event_new(base, STDOUT_FILENO, EV_READ | EV_PERSIST, read_terminal, bev);
    event_add(ev, NULL);

    // loop
    event_base_dispatch(base);

    event_base_free(base);
    event_free(ev);
    bufferevent_free(bev);
    return 0;
}