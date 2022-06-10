/*
@Filename : server.c
@Description : 
@Datatime : 2022/06/10 20:03:22
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include "wrap.h"

#define SERV_PORT 8888

struct client_info { // 描述客户端连接信息
    struct sockaddr_in client_addr;
    int client_fd;
};

void* thread_work(void* arg) {
    // 接收客户端连接的信息
    struct client_info* client_p = (struct client_info*)arg;
    struct sockaddr_in client_addr = client_p -> client_addr;
    int client_fd = client_p -> client_fd;

    char client_IP[1024];
    printf("client connected -- ip : %s port : %d\n", 
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
        ntohs(client_addr.sin_port)
    );
    char buf[4096];
    int read_bytes;
    while (1) {
        read_bytes = Read(client_fd, buf, sizeof(buf));
        if (read_bytes == 0) { // read_bytes==0 意思是读到末尾（客户端关闭了）
            close(client_fd);
            printf("client shutdown  -- ip : %s port : %d\n", 
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)),
                ntohs(client_addr.sin_port)
            );
            pthread_exit(NULL);
        }
        Write(STDOUT_FILENO, buf, read_bytes);
        for (int i = 0; i < read_bytes; ++ i) {
            buf[i] = toupper(buf[i]);
        }
        Write(client_fd, buf, read_bytes);
    }
    return NULL;
}

int main(int argc, char** argv) {
    // 监听socket
    int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    Listen(listen_fd, 128);    
    // 客户端socket
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // 客户端连接信息记录
    struct client_info clients[1024];
    int index = 0;

    pthread_t tid;
    while (1) {
        client_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        // 保存客户端连接信息
        clients[index].client_addr = client_addr;
        clients[index].client_fd = client_fd;
        ++ index;
        // 创建子线程
        int ret = pthread_create(&tid, NULL, thread_work, (void*)&clients[index - 1]);
        if (ret != 0) {
            fprintf(stderr, "pthread_create error : %s\n", strerror(ret));
            exit(-1);
        }
        ret = pthread_detach(tid); // 设置线程分离
        if (ret != 0) {
            fprintf(stderr, "pthread_detach error : %s\n", strerror(ret));
            exit(-1);
        }
    }
    return 0;
}