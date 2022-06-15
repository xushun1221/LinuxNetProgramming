/*
@Filename : epoll_reactor_server.c
@Description : epoll 基于非阻塞IO事件驱动的反应堆模型  libevent的部分源码实现
@Datatime : 2022/06/14 14:23:47
@Author : xushun
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>

#define MAX_EVENTS 1024     // 监听数量上限
#define MAX_BUFSIZE 4096
#define SERV_PORT 8888

void recvdata(int fd, int events, void* arg);
void senddata(int fd, int events, void* arg);

// 描述监听文件描述符相关信息
struct myevent_s {
    int fd;                 // 要监听的文件描述符
    int events;             // 对应的监听事件
    void* arg;              // 泛型参数
    void (*call_back)(int fd, int events, void* arg);
                            // 回调函数
    int status;             // 是否正在被监听 1 - 在监听树上 0 - 不在监听树上
    char buf[MAX_BUFSIZE];
    int len;
    long last_active;       // 记录上次活动时间 即加入监听树的时间
};

int g_epfd;
struct myevent_s g_events[MAX_EVENTS + 1];

// 初始化struct myevent_s成员变量
void eventset(struct myevent_s* ev, int fd, void (*call_back)(int, int, void*), void* arg) {
    ev -> fd = fd;
    ev -> call_back = call_back;
    ev -> events = 0;
    ev -> arg = arg;
    ev -> status = 0;
    memset(ev -> buf, 0, sizeof(ev -> buf));
    ev -> len = 0;
    ev -> last_active = time(NULL);
    return;
}

// 向监听树中添加一个文件描述符
void eventadd(int epfd, int events, struct myevent_s* ev) {
    struct epoll_event epev = {0, {0}};
    epev.data.ptr = ev;
    epev.events = ev -> events = events;

    int op;
    if (ev -> status == 0) {
        op = EPOLL_CTL_ADD;
        ev -> status = 1;
    }

    if (epoll_ctl(epfd, op, ev -> fd, &epev) < 0) {
        printf("event add failed [fd = %d], events[%d]\n", ev -> fd, events);
    } else {
        printf("event add OK [fd = %d], op = %d, events[%0X]\n", ev -> fd, op, events);
    }
    return;
}

void eventdel(int epfd, struct myevent_s* ev) {
    struct epoll_event epv = {0, {0}};

    if (ev -> status != 1) {
        return;
    }
    
    // epv.data.ptr = ev;
    epv.data.ptr = NULL;
    ev -> status = 0; // 修改状态
    epoll_ctl(epfd, EPOLL_CTL_DEL, ev -> fd, &epv); // 从监听树上将ev -> fd 摘下

    return;
}

void recvdata(int fd, int events, void* arg) {
    struct myevent_s *ev = (struct myevent_s*)arg;
    int len;

    len = recv(fd, ev -> buf, sizeof(ev -> buf), 0); // 读文件描述符 数据存入myevent_s 成员buf中

    eventdel(g_epfd, ev); // 该节点从监听树上摘下

    if (len > 0) {
        ev -> len = len;
        ev -> buf[len] = '\0'; // 添加字符串结束标记
        printf("C[%d] : %s\n", fd, ev -> buf);

        eventset(ev, fd, senddata, ev); // 将该fd的回调函数设置为senddata
        eventadd(g_epfd, EPOLLOUT, ev); // 将fd加入监听树 监听写事件
    } else if (len == 0) {
        close(ev -> fd);
        // ev - g_events 地址相减得到偏移元素位置
        printf("[fd=%d] pos[%ld], closed\n", fd, ev - g_events);
    } else {
        close(ev -> fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }
    return;
}

void senddata(int fd, int events, void* arg) {
    struct myevent_s* ev = (struct myevent_s*)arg;
    int len;

    len = send(fd, ev -> buf, ev -> len, 0); // 直接将数据写回给客户端 未作处理 

    eventdel(g_epfd, ev); // 从监听树中摘下fd

    if (len > 0) {
        printf("send[fd = %d], [%d]%s\n", fd, len, ev -> buf);
        eventset(ev, fd, recvdata, ev); // 将该fd的回调函数改为 recvdata
        eventadd(g_epfd, EPOLLIN, ev);  // 重新将其添加到监听树上 改为监听读事件
    } else {
        close(ev -> fd); // 关闭连接
        printf("send[fd = %d] error %s\n", fd, strerror(errno));
    }
    return;
}

// listen_fd的回调函数 与客户端建立连接
void acceptconn(int lfd, int events, void* arg) {
    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);
    int cfd, i;

    if ((cfd = accept(lfd, (struct sockaddr*)&cin, &len)) == -1) {
        if (errno != EAGAIN && errno != EINTR) {

        } 
        printf("%s : accept, %s\n", __func__, strerror(errno));
        return;
    }

    do {
        for (i = 0; i < MAX_EVENTS; ++ i) { // 从全局g_events中找一个空闲元素
            if (g_events[i].status == 0) {
                break; // 跳出for
            }
        }
        if (i == MAX_EVENTS) {
            printf("%s : max connect limit[%d]\n", __func__, MAX_EVENTS);
            break; // 跳出do_while
        }

        int flag = 0;
        if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) { // cfd 设置为nonblocking
            printf("%s : fcntl nonblcoking failed, %s\n", __func__, strerror(errno));
            break;
        }

        // 给cfd设置一个myevent_s结构体 回调函数设置为recvdata
        eventset(&g_events[i], cfd, recvdata, &g_events[i]);
        eventadd(g_epfd, EPOLLIN, &g_events[i]); // cfd添加到监听树中 监听读事件
    } while(0);

    printf("new connect [%s:%d][time:%ld], pos[%d]\n",
        inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active, i
    );
    return;
}

// 初始化listen_fd
void initlistensocket(int epfd, short port) {
    struct sockaddr_in sin;

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(lfd, F_SETFL, O_NONBLOCK); // listen_fd 设为非阻塞

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);

    bind(lfd, (struct sockaddr*)&sin, sizeof(sin));
    listen(lfd, 20);

    // void eventset(struct myevent_s* ev, int fd, void (*call_back)(int, int, void*), void* arg);
    eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
    // void eventadd(int epfd, int events, struct myevent_s* ev);
    eventadd(epfd, EPOLLIN, &g_events[MAX_EVENTS]);

    return;
}



int main(int argc, char** argv) {
    unsigned short port = SERV_PORT;
    
    g_epfd = epoll_create(MAX_EVENTS + 1); // 创建全局监听树
    if (g_epfd <= 0) {
        printf("create epfd in %s err %s\n", __func__, strerror(errno));
    }

    initlistensocket(g_epfd, port); // 初始化监听socket

    struct epoll_event events[MAX_EVENTS + 1];  // 保存已满足就绪事件的文件描述符数组

    printf("server running : port[%d]\n", port);

    int checkpos = 0;
    while (1) {
        // 超时验证 每次测试100个连接，不测试listen_fd 当客户端60秒内没有和服务器通信 则关闭此客户端连接
        long now = time(NULL); // 当前时间
        for (int i = 0; i < 100; ++ i, ++ checkpos) { // 每次循环检测100个 使用checkpos控制检测对象
            if (checkpos == MAX_EVENTS) {
                checkpos = 0;
            }
            if (g_events[checkpos].status != 1) { // 不在监听树上
                continue;
            }

            long duration = now - g_events[checkpos].last_active; // 距离上次活跃过去的时间

            if (duration >= 60) {
                close(g_events[checkpos].fd);   // 关闭与该客户端的连接
                printf("fd[%d] timeout\n", g_events[checkpos].fd);
                eventdel(g_epfd, &g_events[checkpos]);  // 将客户端从监听树上摘下
            }
        }
        //
 
        // 监听树将就绪的事件的文件描述符添加至events数组中返回 1s内没有事件就绪 返回0
        int nfd = epoll_wait(g_epfd, events, MAX_EVENTS + 1, 1000);
        if (nfd < 0) {
            printf("epoll_wait error, exit\n");
            break;
        }

        for (int i = 0; i < nfd; ++ i) {
            // 使用自定义的struct myevent_s类型指针 接收联合体data的void* ptr成员
            struct myevent_s* ev = (struct myevent_s*)events[i].data.ptr;

            if ((events[i].events & EPOLLIN) && (ev -> events & EPOLLIN)) { // 读事件就绪
                ev -> call_back(ev -> fd, events[i].events, ev -> arg);
            }
            if ((events[i].events & EPOLLOUT) && (ev -> events & EPOLLOUT)) { // 写事件就绪
                ev -> call_back(ev -> fd, events[i].events, ev -> arg);
            }
        }
    }
    return 0;
}