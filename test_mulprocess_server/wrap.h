/*
@Filename : wrap.h
@Description : 
@Datatime : 2022/06/10 14:37:17
@Author : xushun
*/
#ifndef __WRAP_H_
#define __WRAP_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void perr_exit(const char* s);
int Accept(int fd, struct sockaddr* sa, socklen_t* salen); // 和accept相比只有一个字母变大写 可以在vim中跳转manpages
int Bind(int fd, const struct sockaddr* sa, socklen_t salen);
int Connect(int fd, const struct sockaddr* sa, socklen_t salen);
int Listen(int fd, int backlog);
int Socket(int family, int type, int protocol);
ssize_t Read(int fd, void* buf, size_t nbytes);
ssize_t Write(int fd, const void* buf, size_t nbytes);
int Close(int fd);
ssize_t Readn(int fd, void* buf, size_t n);
ssize_t Writen(int fd, const void* buf, size_t n);
ssize_t my_read(int fd, char* ptr);
ssize_t Readline(int fd, void* buf, size_t maxlen);

#endif