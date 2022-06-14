/*
@Filename : test_client.c
@Description : 
@Datatime : 2022/06/14 09:02:48
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "wrap.h"

#define SERV_PORT 8888
#define SERV_IP "127.0.0.1"
#define MAX_BUFSIZE 10

int main(int argc, char** argv) {
    int client_fd = Socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);
    Connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    int read_bytes;
    char buf[MAX_BUFSIZE];
    char ch = 'a';
    while (1) {
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
        ++ch;
        Write(client_fd, buf, MAX_BUFSIZE);
        sleep(5);
    }
    Close(client_fd);
    return 0;
}