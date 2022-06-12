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
    int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);  
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    Listen(listen_fd, 128);
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd;
    char client_ip[INET_ADDRSTRLEN]; // #define INET_ADDRSTRLEN 16
    char buf[4096];
    int read_bytes;

    fd_set all_rset, ret_rset; 
    FD_ZERO(&all_rset);
    FD_SET(listen_fd, &all_rset); 
    // -----------------
    int clients[FD_SETSIZE]; // 自定义数组clients 防止遍历1024个fd FD_SETSIZE 默认为1024
    int max_idx = -1; // clients 顶端元素的下标
    for (int i = 0; i < FD_SETSIZE; ++ i) {
        clients[i] = -1;
    }
    // ------------------
    int ret_nready; 
    int max_fd = listen_fd; 
    while (1) {
        ret_rset = all_rset;
        ret_nready = select(max_fd + 1, &ret_rset, NULL, NULL, 0); 
        if (ret_nready < 0) {
            perr_exit("select error");
        } else if (ret_nready > 0) { 
            if (FD_ISSET(listen_fd, &ret_rset)) { 
                client_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                printf("client connected -- ip : %s port : %d\n", 
                    inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                    ntohs(client_addr.sin_port)
                );
                FD_SET(client_fd, &all_rset);

                // --------------
                int i;
                for (i = 0; i < FD_SETSIZE; ++ i) { // 找clients里第一个未使用的位置（-1）
                    if (clients[i] == -1) {
                        clients[i] = client_fd;
                        break;
                    }
                }
                if (i == FD_SETSIZE) { // 到达select能监听的fd的上限
                    fputs("too many clients\n", stderr);
                    exit(-1);
                }
                if (i > max_idx) { // 保证max_idx是clients最后一个元素的下标
                    max_idx = i;
                }
                // --------------

                if (client_fd > max_fd) { 
                    max_fd = client_fd;
                }
                if (-- ret_nready == 0) { 
                    continue;
                }
            }
            // -------------------------------------
            for (int i = 0; i <= max_idx; ++ i) { // 检测哪个fd监听到事件
                int fd;
                if ((fd = clients[i]) < 0) {
                    continue;
                }
                if (FD_ISSET(fd, &ret_rset)) {
                    read_bytes = Read(fd, buf, sizeof(buf));
                    if (read_bytes == 0) {
                        Close(fd);
                        FD_CLR(fd, &all_rset);
                        clients[i] = -1;
                        printf("client closed    -- ip : %s port : %d\n", 
                            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                            ntohs(client_addr.sin_port)
                        );
                    } else if (read_bytes > 0) {
                        Write(STDOUT_FILENO, buf, read_bytes); 
                        for (int j = 0; j < read_bytes; ++ j) {
                            buf[j] = toupper(buf[j]);
                        }
                        Write(fd, buf, read_bytes); 
                    }
                    if (-- ret_nready == 0) {
                        break;
                    }
                }
            }
            // ---------------------------------
        }
    }
    Close(listen_fd);
    return 0;
}