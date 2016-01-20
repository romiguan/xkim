#ifndef __IO_CHANNEL_H__
#define __IO_CHANNEL_H__

#include <stdint.h>

#ifdef UID_TYPE_64
    typedef int64_t UID;
#else
    typedef int UID;
#endif

namespace io
{

struct IoChannel
{
    //用户ID
    UID _uid;
    //io线程ID
    int _io;
    //链接ID
    uint64_t _conn;
    //高32位:ip，低32位:port
    uint64_t _host;
};

}

#endif
