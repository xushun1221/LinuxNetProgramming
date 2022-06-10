/*
@Filename : server.c
@Description : 
@Datatime : 2022/06/10 18:32:28
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include "wrap.h"

#define SERV_PORT 8888

void catch_child(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0) { // 回收到子进程
        
    }
    return;
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
    // 注册捕捉函数
    struct sigaction act, oldact;
    act.sa_handler = catch_child;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGCHLD, &act, &oldact) != 0) {
        perr_exit("sigaction error");
    }
    // 客户端socket
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    pid_t pid;

    while (1) {
        client_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        pid = fork();
        if (pid < 0) {
            perr_exit("fork error");
        } else if (pid == 0) { // child process
            close(listen_fd);
            break; // 子进程不监听 跳出循环 执行业务逻辑
        } else { // parent process
            close(client_fd); // 父进程不和客户端通信 关闭client_fd 继续监听
        }
    }

    if (pid == 0) { // 子进程逻辑
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
                exit(1);
            }
            Write(STDOUT_FILENO, buf, read_bytes);
            for (int i = 0; i < read_bytes; ++ i) {
                buf[i] = toupper(buf[i]);
            }
            Write(client_fd, buf, read_bytes);
        }
    }
    return 0;
}