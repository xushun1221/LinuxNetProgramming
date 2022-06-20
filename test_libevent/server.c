/*
@Filename : server.c
@Description : libevent TCP server
@Datatime : 2022/06/20 20:17:12
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#define SERV_PORT 8888
#define MAX_BUFSIZE 1024

void read_cb(struct bufferevent* bev, void* ctx) {
    char buf[MAX_BUFSIZE] = {0};
    bufferevent_read(bev, buf, sizeof(buf));
    printf("client : %s\n", buf);
    sprintf(buf, "server : got your message\n");
    bufferevent_write(bev, buf, strlen(buf) + 1);
    return;
}

void write_cb(struct bufferevent* bev, void* ctx) {
    printf("server : reply client success, callback called\n");
    return;
}

void event_cb(struct bufferevent* bev, short events, void* ctx) {
    if (events & BEV_EVENT_EOF) {
        printf("client closed\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("bufferevent error\n");
    } else {
        return;
    }
    bufferevent_free(bev);
    printf("bufferevent free\n");
    return;
}

// listener callback
void listener_cb(
    struct evconnlistener* listener, evutil_socket_t sock, 
    struct sockaddr* addr, int len, void* ptr
) {
    printf("new client connected\n");
    // get event_base from arg
    struct event_base* base = (struct event_base*)ptr;
    // new bufferevent
    struct bufferevent* bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
    // set callback function for buffer
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
    // enable read_buffer
    bufferevent_enable(bev, EV_READ);
    return;
}

int main(int argc, char** argv) {
    // server socket
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    // base
    struct event_base* base = event_base_new();
    // socket bind listen accept
    struct evconnlistener* listener = evconnlistener_new_bind(
        base, listener_cb, base,
        LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
        -1, (struct sockaddr*)&serv_addr, sizeof(serv_addr)
    );
    // loop
    event_base_dispatch(base);
    // free
    evconnlistener_free(listener);
    event_base_free(base);
    return 0;
}