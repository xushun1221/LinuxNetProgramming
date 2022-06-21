/*
@Filename : server.h
@Description : header of server
@Datatime : 2022/06/21 15:33:55
@Author : xushun
*/
#ifndef __SERVER_H_
#define __SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 1000

int init_listen_fd(int port, int epfd);
void epoll_run(int port);
void do_accept(int listen_fd, int epfd);
void do_read(int client_fd, int epfd);
int get_line(int sock, char * buf, int size);
void disconnect(int client_fd, int epfd);
void http_request(const char* request, int client_fd);
void send_responsd_head(int client_fd, int no, const char* description, const char* type, long len);
void send_file(int client_fd, const char* filename);
void send_dir(int client_fd, const char* dirname);
void send_error(int client_fd, int status, char* title, char* text);
int hexit(char c);
void encode_str(char* to, int tosize, const char* from);
void decode_str(char* to, char* from);
const char* get_file_type(const char* filename);

#endif