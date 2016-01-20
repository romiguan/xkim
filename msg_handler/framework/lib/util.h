#ifndef __UTIL_H__
#define __UTIL_H__

#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <string>

namespace util
{

#if defined(__GNUC__)
    #define expect_true(v) __builtin_expect((v), 1)
    #define expect_false(v) __builtin_expect((v), 0)
#else
    #warning "expect_true and expect_false will do nothing"
    #define expect_true(v) (v)
    #define expect_false(v) (v)
#endif

struct MessageIdDetail 
{
    unsigned int _time;
    unsigned int _rand1:12;
    unsigned int _tid:20;
    unsigned int _host_id;
    unsigned int _counter:16;
    unsigned int _rand2:16;
};

union MessageId
{
    MessageIdDetail _id;
    unsigned char _bytes[16];
    unsigned long long _long[2];
};

class Counter
{
    public:
        Counter();
        ~Counter() {}

        unsigned int get();

    private:
        unsigned int _count;
        time_t _second;
        char _pad[128];
};

union HostIp
{
    unsigned int _ip;
    struct
    {
        unsigned char _ip0;
        unsigned char _ip1;
        unsigned char _ip2;
        unsigned char _ip3;
    };
};

unsigned int gethost(const char* eth);
unsigned long long murmurHash64B(const void* key, int len, unsigned int seed);

inline bool setNonblocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        return false;
    return true;
}

inline unsigned int bkdrHash(const char* str, int len)
{
    unsigned int seed = 131;
    unsigned int hash = 0;
    for (int i = 0; i < len; ++i)
        hash = hash * seed + (*str++);
    return (hash & 0x7FFFFFFF);
}

inline pid_t gettid()
{
    return syscall(SYS_gettid);
}

uint64_t buildServiceID(const char* eth, unsigned short port);

std::string Uint64ToString(uint64_t value);
std::string Int64ToString(int64_t value);

}

#endif
