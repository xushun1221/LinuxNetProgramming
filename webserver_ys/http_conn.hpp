/*
@Filename : http_conn.hpp
@Description : http connection header
@Datatime : 2022/06/24 19:39:40
@Author : xushun
*/
#ifndef __HTTP_CONN_H_
#define __HTTP_CONN_H_

#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <stdarg.h>

class http_conn {
public:
    static const int FILENAME_LEN = 200;        //文件名的最大长度
    static const int READ_BUFFER_SIZE = 2048;   // 读缓冲区的大小
    static const int WRITE_BUFFER_SIZE = 1024;  // 写缓冲区的大小
    // HTTP 请求方法 我们的服务器仅支持GET
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATHC};
    // 解析客户请求时 主状态机所处的状态
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, // 分析请求行
                       CHECK_STATE_HEADER,          // 分析头部字段
                       CHECK_STATE_CONTENT };       // 分析内容
    // 行的读取状态
    enum LINE_STATUS { LINE_OK = 0,      // 读到一个完整的行
                       LINE_BAD,         // 行出错
                       LINE_OPEN };      // 行数据不完整
    // 处理HTTP请求时的可能结果
    enum HTTP_CODE { NO_REQUEST,         // 请求不完整 需要继续读客户端数据
                    GET_REQUEST,         // 获得了一个完整的客户请求
                    BAD_REQUEST,         // 客户请求有语法错误
                    NO_RESOURCE,         // 目标文件不存在
                    FORBIDDEN_REQUEST,   // 客户对资源没有足够的访问权限 
                    FILE_REQUEST,        // 获取文件成功
                    INTERNAL_ERROR,      // 服务器内部错误
                    CLOSED_CONNECTION }; // 客户端已经关闭连接
public:
    http_conn() {}
    ~http_conn() {}
public:
    // 初始化新接收的连接
    void init(int sockfd, const sockaddr_in& addr);
    // 关闭连接
    void close_conn(bool real_close = true);
    // 处理客户请求
    void process();
    // 非阻塞读操作
    bool read();
    // 非阻塞写操作
    bool write();
public:
    // 初始化连接
    void init();
    // 解析HTTP请求
    HTTP_CODE process_read();
    // 填充HTTP应答
    bool process_write(HTTP_CODE ret);
    // 下面这一组函数被process_read调用以分析HTTP请求
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();
    // 下面这一组函数被process_write调用以填充HTTP应答
    void unmap();
    bool add_response(const char* format, ...);
    bool add_content(const char* content);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
public:
    // 所有socket上的事件都被注册到同一个epoll内核事件表中 
    // 所以epoll文件描述符设置为静态
    static int m_epollfd;
    // 统计用户数量
    static int m_user_count;
private:
    // 该HTT连接的socket和对方的地址
    int m_sockfd;
    struct sockaddr_in m_address;
    
    // 读缓冲区
    char m_read_buf[READ_BUFFER_SIZE];
    // 读缓冲区中已经读入的客户数据的最后一个字节的下一个位置
    int m_read_idx;
    // 当前正在分析的字符在读缓冲区中的位置
    int m_checked_idx;
    // 当前正在解析的行的起始位置
    int m_start_line;
    // 写缓冲区
    char m_write_buf[WRITE_BUFFER_SIZE];
    // 写缓冲区中待发送的字节数
    int m_write_idx;

    // 主状态机当前所处的状态
    CHECK_STATE m_check_state;
    // 请求方法
    METHOD m_method;

    // 客户请求目标文件的完整路径
    // 内容为 doc_root + m_url doc_root为网站根目录
    char m_real_file[FILENAME_LEN];
    // 客户请求目标文件的文件名
    char* m_url;
    // HTTP的版本号 我们仅支持HTTP/1.1
    char* m_version;
    // 主机名
    char* m_host;
    // HTTP请求的消息体的长度
    int m_content_length;
    // HTTP请求是否要求保持连接
    bool m_linger;

    // 客户请求的目标文件被mmap到内存中的位置
    char* m_file_address;
    // 目标文件的状态 可以获得文件是否存在 是否为目录文件 是否可读 文件大小等信息
    struct stat m_file_stat;
    // 我们将采用writev来执行写操作 m_iv_count表示被写内存块的数量
    struct iovec m_iv[2];
    int m_iv_count;
};
#endif