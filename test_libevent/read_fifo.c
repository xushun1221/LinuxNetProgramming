/*
@Filename : read_fifo.c
@Description : test read fifo
@Datatime : 2022/06/20 10:51:23
@Author : xushun
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <event2/event.h>
#include <sys/stat.h>
#include <fcntl.h>

void read_cb(evutil_socket_t fd, short what, void* arg) {
    char buf[4096] = {0};
    int read_bytes = read(fd, buf, sizeof(buf));
    printf("read event: %s, ", what & EV_READ ? "Yes" : "No");
    printf("data len = %d, buf = %s\n", read_bytes, buf);
    sleep(1);
    return;
}

int main(int argc, char** argv) {
    unlink("myfifo");
    mkfifo("myfifo", 0644);
    int fd = open("myfifo", O_RDONLY | O_NONBLOCK);
    struct event_base* base = event_base_new();
    struct event* ev = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, NULL);
    // struct event* ev = event_new(base, fd, EV_READ, read_cb, NULL);
    event_add(ev, NULL);
    event_base_dispatch(base);
    event_free(ev);
    event_base_free(base);
    close(fd);
    unlink("myfifo");
    return 0;
}