#ifndef __MS_MESSAGE_H__
#define __MS_MESSAGE_H__

#include <sys/time.h>
#include <stdint.h>

namespace ms
{

extern int magic;

enum MESSAGE_TYPE
{
    TYPE_UNKNOWN = 0,
    APP_REQUEST = 1,
    APP_REPLY,
};

enum MESSAGE_SOURCE
{
    UNKNOWN = 0,
    APP = 1,
    BACKEND,
};

struct IMessage
{
    char* _b;               // 数据buffer,原始的frame
    char* _data;            // 有效数据开始
    int _size;              // 数据buffer大小
    int _data_len;          // 有效数据长度
    int _frame_len;         // frame长度

    int _state;             // message状态机,来自底层的io_app_connection
    int _uid;               // message对应的用户
    int _io;                // message来自哪个io线程/去往哪个io线程
    uint64_t _conn;         // message来自哪个链接/去往哪个链接
    char _id[8];            // message id

    unsigned int _priority:4;        // message的优先级
    unsigned int _need_collect:1;    // push失败后,是否需要收集
    unsigned int _force_push:1;      // 如果push队列满,是否强制进队列
    unsigned int _close_conn:1;      // 关闭连接,数据丢弃
    unsigned int _type:2;            // message type
    unsigned int _from:2;            // message的来源
    unsigned int _to:2;              // message的目的地
    void (*_push_callback)(void* v);
    void (*_switch_state)(void* v, int s);

    struct timeval _gen_time;
    struct timeval _process_begin_time;
    struct timeval _process_end_time;

    IMessage* _next;
};

/*
 * @f: 通过原始frame构造message
 * @b,@len: 原始frame的地址和长度
 */
IMessage* createIMessage(const char* b, int len);

/*
 * @f: 用来构造回写的frame
 * @len: valid data length
 */
IMessage* createIMessage(int len);

/*
 * @f: 用来构造回写的frame
 * @len: valid data length
 */
IMessage* fixIMessage(IMessage* msg, int len);
void releaseIMessage(void* v);

}

void switch_state_app_msg(void* v, int);

#endif
