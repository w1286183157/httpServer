#pragma once
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include "locker.h"
#include <stdarg.h>
#include <assert.h>
#include <fcntl.h>

class http_conn
{
public:
    /*文件名的最大长度*/
    static const int FILENAME_LEN = 200;
    /*读缓冲区的大小*/
    static const int READ_BUFFER_SIZE = 2048;
    /*写缓冲区的大小*/
    static const int WRITE_BUFFER_SIZE = 1024;
    /*http 请求方法 目前只支持get*/
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTION,
        CONNECT,
        PATCH
    };
    /*解析客户请求时，主状态机所处的状态*/
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    /*服务器处理http请求的可能结果*/
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    /*行的读取状态*/
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

private:
    int m_sockfd; // http连接的socket和对方的socket地址
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE];   //读缓冲区
    int m_read_idx;                      //表示读缓冲中已经读入的客户数据的最后一个字节的下一个位置
    int m_checked_idx;                   //当前正在分析的字符在读缓冲中的位置
    int m_start_line;                    //当前正在解析行的起始位置
    char m_write_buf[WRITE_BUFFER_SIZE]; //写缓冲区
    int m_write_idx;                     //写缓冲区中待发送的字节数
    CHECK_STATE m_check_state;           //主状态机当前所处的状态
    METHOD m_method;                     //请求方法
    char m_real_file[FILENAME_LEN];      //客户请求的目标文件的完整路径 doc_root+m_url
    char *m_url;                         //客户请求的目标文件名
    char *m_version;                     // http协议版本号，支持http/1.1
    char *m_host;                        //主机名
    int m_content_length;                // http请求的消息体长度
    bool m_linger;                       // http请求是否要保持连接

    char *m_file_address;    //客户端请求的目标文件被mmap内存中的起始位置
    struct stat m_file_stat; //目标文件状态。通过他我们可以判断文件是否存在、是否为目录、是否可读并获取文件大小等信息
    struct iovec m_iv[2];    //采用writev来执行写操作，其中m_iv_count表示被写内存块的数量
    int m_iv_count;

private:
    /* data */
    void init();              //初始化连接
    HTTP_CODE process_read(); //解析http请求
    bool process_write(HTTP_CODE ret);

    /*被process_write 调用分析http请求*/
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();

    char *get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();

    /*被process_write调用填充http应答*/
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;    //所有的socket被注册到一个epoll内核事件表中
    static int m_user_count; //统计用户数量
public:
    http_conn(/* args */);
    ~http_conn();

public:
    void init(int sockfd, const sockaddr_in &addr); //初始化并接受新连接
    void close_conn(bool read_close = true);        //关闭连接
    void process();                                 //处理客户请求
    bool read();                                    //非阻塞读操作
    bool write();                                   //非阻塞写操作
};

http_conn::http_conn(/* args */)
{
}

http_conn::~http_conn()
{
}
