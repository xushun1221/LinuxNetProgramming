/*
@Filename : server.c
@Description : test poll server
@Datatime : 2022/06/12 19:22:06
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <poll.h>
#include "wrap.h"

#define SERV_PORT 8888
#define MAX_CLIENTS 1024

int main(int argc, char** argv) {
    // listen socket
    int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);  // listen_fd == 3
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // **
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr)); // **
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    Listen(listen_fd, 128);
    // client socket
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;
    char client_ip[INET_ADDRSTRLEN];
    // read buffer
    char buf[4096];
    int read_bytes;
    // 
    struct pollfd clients[MAX_CLIENTS]; // 监听的client数组
    for (int i = 0; i < MAX_CLIENTS; ++ i) {
        clients[i].fd = -1; // 初始化 fd = -1 表示空
    }
    clients[0].fd = listen_fd; // 第一个要监听的是listen_fd
    clients[0].events = POLLIN; // 监听读事件
    int max_idx = 0; // 有效元素最大下标

    int ret_nready; // 接收poll返回值 记录满足监听事件的fd个数 

    while (1) {
        ret_nready = poll(clients, max_idx + 1, - 1); // -1 表示阻塞监听
        if (ret_nready < 0) {
            perr_exit("poll error");
        } else if (ret_nready > 0) {
            if (clients[0].revents & POLLIN) { // listen_fd有读事件 处理新连接 
                client_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                printf("client connected -- ip : %s port : %d\n", 
                    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                    ntohs(client_addr.sin_port)
                );
                int i;
                for (i = 1; i < MAX_CLIENTS; ++ i) { // clients[0] == listen_fd
                    if (clients[i].fd == -1) {
                        clients[i].fd = client_fd; // 找到clients中的空闲位置放新的client_fd
                        clients[i].events = POLLIN;
                        break;
                    }
                }
                if (i == MAX_CLIENTS) { // 没找到空位 说明客户端太多
                    fputs("too many clients\n", stderr);
                    exit(-1);
                }
                if (i > max_idx) { // 更新最大有效下标
                    max_idx = i;
                }
                if (-- ret_nready == 0) { // 没有更多事件 返回poll阻塞
                    continue;
                }
            }
            // 还有事件要处理
            for (int i = 1; i <= max_idx; ++ i) { // 处理需要处理的fd
                if (clients[i].fd == -1) {
                    continue;
                }
                if (clients[i].revents & POLLIN){ // 有读事件
                    read_bytes = Read(clients[i].fd, buf, sizeof(buf));
                    if (read_bytes == 0) { // 客户端关闭
                        Close(clients[i].fd);
                        clients[i].fd = -1; // 在poll中取消监听某个fd 直接在监听数组中设为-1即可
                        printf("client closed    -- ip : %s port : %d\n", 
                            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                            ntohs(client_addr.sin_port)
                        );
                    } else if (read_bytes > 0) {
                        Write(STDOUT_FILENO, buf, read_bytes);
                        for (int j = 0; j < read_bytes; ++ j) {
                            buf[j] = toupper(buf[j]);
                        }
                        Write(clients[i].fd, buf, read_bytes);
                    } else { // read_bytes < 0 出错
                        if (errno == ECONNRESET) { // 收到RST 连接被重置
                            Close(clients[i].fd);
                            clients[i].fd = -1;
                            printf("client reset     -- ip : %s port : %d\n", 
                                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                                ntohs(client_addr.sin_port)
                            );
                        } else {
                            perr_exit("read error");
                        }
                    }
                }
                if (-- ret_nready == 0) {
                    break; // 不存在其他事件需要处理 退出该循环
                }
            }
        }
    }
    Close(listen_fd);
    return 0;
}