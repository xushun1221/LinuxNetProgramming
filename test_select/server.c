/*
@Filename : server.c
@Description : select server test
@Datatime : 2022/06/12 10:04:34
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
    char client_ip[1024];
    // read buffer
    char buf[4096];
    int read_bytes;

    fd_set all_rset, ret_rset; // all_rset 保存所有需要读监听的fd ret_rset 保存select返回的读fd集合
    FD_ZERO(&all_rset);
    FD_SET(listen_fd, &all_rset); // listen_fd 监听新的客户端连接 添加到读监听集合中
    
    int ret_nready; // 保存select返回的满足监听条件的fd数
    int max_fd = listen_fd; // 当前监听的最大的fd
    while (1) {
        ret_rset = all_rset;
        ret_nready = select(max_fd + 1, &ret_rset, NULL, NULL, 0); // 只监听读事件 非阻塞轮询
        if (ret_nready < 0) {
            perr_exit("select error");
        } else if (ret_nready > 0) { // 有监听事件发生
            if (FD_ISSET(listen_fd, &ret_rset)) { // listen_fd 有读事件发生 即有客户端连接请求
                client_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                printf("client connected -- ip : %s port : %d\n", 
                    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                    ntohs(client_addr.sin_port)
                );
                FD_SET(client_fd, &all_rset); // 新的客户端fd需要监听
                if (client_fd > max_fd) { // 更新当前监听的最大的fd
                    max_fd = client_fd;
                }
                // 如果只有一个事件 且为listen_fd的 则不需要遍历其他监听的client_fd
                if (-- ret_nready == 0) { 
                    continue;
                }
            }
            // 除了listen_fd之外 还有读事件发生 处理其他在ret_rset中的client_fd
            for (int fd = listen_fd + 1; fd <= max_fd; ++ fd) {
                if (FD_ISSET(fd, &ret_rset)) { // 该client_fd有读事件
                    read_bytes = Read(fd, (void*)&buf, sizeof(buf));
                    if (read_bytes == 0) { // client关闭了
                        Close(fd);
                        printf("client closed\n");
                        FD_CLR(fd, &all_rset); // 不需要再监听了
                    } else if (read_bytes > 0) { // 读到数据 处理
                        Write(STDOUT_FILENO, buf, read_bytes); // print
                        for (int i = 0; i < read_bytes; ++ i) {
                            buf[i] = toupper(buf[i]);
                        }
                        Write(fd, buf, read_bytes); // upper chars send to client
                    }
                    if (-- ret_nready == 0) { // 处理完了
                        break;
                    }
                }
            }
        }
    }
    Close(listen_fd);
    return 0;
}