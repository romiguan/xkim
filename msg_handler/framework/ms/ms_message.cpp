#include "ms_message.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include <glog/logging.h>

#define M_FRAME_BYTES sizeof(int)
#define M_MAGIC_BYTES sizeof(int)

#define M_FRAME_DATA(x) \
    ((x)+M_FRAME_BYTES+M_MAGIC_BYTES)

//根据frame size得到data size
#define M_FRAME_DATA_SIZE(x) \
    ((x)-M_FRAME_BYTES-M_MAGIC_BYTES)

//根据data size得到frame size
#define M_FRAME_SIZE(x) \
    ((x)+M_FRAME_BYTES+M_MAGIC_BYTES)

namespace ms
{

int magic = 0x696D696D;

/*
 * frame:
 * |int(后面数据长度)|int(magic)|data(实际数据)
 */

/*
 * 通过原始frame构造message
 * 原始frame的地址和长度
 */
IMessage* createIMessage(const char* b, int len)
{
    int want = len + 32;
    IMessage* m = (IMessage*)malloc(sizeof(IMessage));
    char* buf = (char*)malloc(want);
    if (m == 0 || buf == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }
    memcpy(buf, b, len);
    m->_b = buf;
    m->_data = M_FRAME_DATA(buf);
    m->_size = want;
    m->_data_len = M_FRAME_DATA_SIZE(len);
    m->_frame_len = len;

    m->_uid = -1;
    m->_io = -1;
    m->_conn = -1;

    m->_priority = 15;
    m->_need_collect = 0;
    m->_force_push = 0;
    m->_close_conn = 0;
    m->_type = 0;
    m->_push_callback = 0;
    m->_switch_state = 0;
    m->_next = 0;

    return m;
}

/*
 * 用来构造回写的frame
 * len: valid data length
 */
IMessage* createIMessage(int len)
{
    int want = M_FRAME_SIZE(len);
    IMessage* m = (IMessage*)malloc(sizeof(IMessage));
    char* buf = (char*)malloc(want);
    if (m == 0 || buf == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }
    int* frame = (int*)buf;
    frame[0] = htonl(len+M_MAGIC_BYTES);
    frame[1] = magic;

    m->_b = buf;
    m->_data = M_FRAME_DATA(buf);
    m->_size = want;
    m->_data_len = len;
    m->_frame_len = want;

    m->_uid = -1;
    m->_io = -1;
    m->_conn = -1;

    m->_priority = 15;
    m->_need_collect = 0;
    m->_force_push = 0;
    m->_close_conn = 0;
    m->_type = 0;
    m->_push_callback = 0;
    m->_switch_state = 0;
    m->_next = 0;

    return m;
}

/*
 * 用来构造回写的frame
 * len: valid data length
 */
IMessage* fixIMessage(IMessage* msg, int len)
{
    int want = M_FRAME_SIZE(len);
    if (msg->_size >= want)
    {
        int* frame = (int*)(msg->_b);
        frame[0] = htonl(len+M_MAGIC_BYTES);
        frame[1] = magic;

        msg->_data_len = len;
        msg->_frame_len = want;
    }
    else
    {
        free(msg->_b);

        char* buf = (char*)malloc(want);
        if (buf == 0)
        {
            LOG(ERROR) << "memory error, abort\n";
            abort();
        }
        int* frame = (int*)buf;
        frame[0] = htonl(len+M_MAGIC_BYTES);
        frame[1] = magic;

        msg->_b = buf;
        msg->_data = M_FRAME_DATA(buf);
        msg->_size = want;

        msg->_data_len = len;
        msg->_frame_len = want;
    }

    return msg;
}

void releaseIMessage(void* v)
{
    IMessage* m = (IMessage*)v;
    if (m)
    {
        if (m->_b)
            free(m->_b);
        free(m);
    }
}

}
