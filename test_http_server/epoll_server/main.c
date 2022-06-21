/*
@Filename : main.c
@Description : http server by epoll
@Datatime : 2022/06/21 15:53:08
@Author : xushun
*/
#include "server.h"

int main(int argc, char** argv) {
    // 检查输入的参数
    if (argc < 3) {
        printf("eg: ./server port paht\n");
        exit(1);
    }
    // 采用指定的端口
    int port = atoi(argv[1]);
    // 切换工作目录
    int ret = chdir(argv[2]);
    if (ret == -1) {
        perror("chdir error");
        exit(1);
    }
    // 启动epoll模型
    epoll_run(port);
    return 0;
}