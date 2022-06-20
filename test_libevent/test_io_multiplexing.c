/*
@Filename : test_io_multiplexing.c
@Description : test io multiplexing
@Datatime : 2022/06/20 09:11:53
@Author : xushun
*/
#include <stdio.h>
#include <event2/event.h>

int main(int argc, char** argv) {
    //struct event_base* base = event_new();
    const char** buf = event_get_supported_methods();
    for (int i = 0; i < 5; ++ i) {
        printf("buf[i] = %s\n", buf[i]);
    }
    return 0;
}