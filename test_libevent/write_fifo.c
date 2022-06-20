/*
@Filename : write_fifo.c
@Description : test write fifo
@Datatime : 2022/06/20 10:52:10
@Author : xushun
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/event.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void write_cb(evutil_socket_t fd, short what, void* arg) {
    char buf[4096];
    static int num = 0;
    sprintf(buf, "hello %d", num ++);
    write(fd, buf, strlen(buf) + 1); // sprintf 自动加 \0
    sleep(1);
    return;
}

int main(int argc, char** argv) {
    int fd = open("myfifo", O_WRONLY);
    struct event_base* base = event_base_new();
    struct event* ev = event_new(base, fd, EV_WRITE | EV_PERSIST, write_cb, NULL);
    // struct event* ev = event_new(base, fd, EV_WRITE, write_cb, NULL);
    event_add(ev, NULL);
    event_base_dispatch(base);
    event_free(ev);
    event_base_free(base);
    close(fd);
    return 0;
}