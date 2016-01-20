#include "util.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <algorithm> //for std::reverse

namespace util
{

Counter::Counter():
    _count(0),
    _second(time(0))
{
}

unsigned int Counter::get()
{
    time_t t = time(0);
    if (t != _second)
    {
        _count = 1;
        _second = t;
        return _count;
    }

    ++_count;
    return _count;
}

unsigned long long murmurHash64B(const void* key, int len, unsigned int seed)
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    unsigned int h1 = seed ^ len;
    unsigned int h2 = 0;

    const unsigned int* data = (const unsigned int*)key;

    while(len >= 8)
    {
        unsigned int k1 = *data++;
        k1 *= m; k1 ^= k1 >> r; k1 *= m;
        h1 *= m; h1 ^= k1;
        len -= 4;

        unsigned int k2 = *data++;
        k2 *= m; k2 ^= k2 >> r; k2 *= m;
        h2 *= m; h2 ^= k2;
        len -= 4;
    }

    if(len >= 4)
    {
        unsigned int k1 = *data++;
        k1 *= m; k1 ^= k1 >> r; k1 *= m;
        h1 *= m; h1 ^= k1;
        len -= 4;
    }

    switch(len)
    {
        case 3: h2 ^= ((unsigned char*)data)[2] << 16;
        case 2: h2 ^= ((unsigned char*)data)[1] << 8;
        case 1: h2 ^= ((unsigned char*)data)[0];
                h2 *= m;
    };

    h1 ^= h2 >> 18; h1 *= m;
    h2 ^= h1 >> 22; h2 *= m;
    h1 ^= h2 >> 17; h1 *= m;
    h2 ^= h1 >> 19; h2 *= m;

    unsigned long long h = h1;

    h = (h << 32) | h2;

    return h;
}

unsigned int gethost(const char* eth)
{
    HostIp addr;
    if (eth != 0)
    {
        struct ifreq iface;
        strcpy(iface.ifr_ifrn.ifrn_name, eth);
        int fd = socket(AF_INET, SOCK_DGRAM, 0); 
        if (ioctl(fd, SIOCGIFADDR, &iface) == 0)
        {
            close(fd);
            addr._ip = (int)((struct sockaddr_in *)(&iface.ifr_ifru.ifru_addr))->sin_addr.s_addr;
        }
    }
    else
    {
        struct ifaddrs* ifAddrStruct=NULL;
        getifaddrs(&ifAddrStruct);

        while (ifAddrStruct != NULL)
        {
            if (ifAddrStruct->ifa_addr->sa_family == AF_INET)
            {
                addr._ip = (int)((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr.s_addr;
                if (addr._ip0 != 127)
                    break;
            }

            ifAddrStruct = ifAddrStruct->ifa_next;
        }
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr._ip, ip, sizeof(ip));
    fprintf(stderr, "host: %s(%u) - %d:%d:%d:%d\n", ip,
            addr._ip,
            addr._ip0,
            addr._ip1,
            addr._ip2,
            addr._ip3);

    return addr._ip;
}

uint64_t buildServiceID(const char* eth, unsigned short port)
{
    unsigned int ip = gethost(eth);

    uint64_t id = (((uint64_t)ip) << 32);
    id += port;
    return id;
}

std::string Uint64ToString(uint64_t value)
{
    uint64_t i = value;
    char buf[32]; // 20-digits for uint64_t
    buf[31] = '\0';
    char* p = buf;

    do {
        int32_t lsd = i % 10;
        i /= 10;
        *p++ = '0' + lsd;
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);
    return std::string(buf);
}

static char* LLToStr(int64_t value, char* ptr)
{
    int64_t t;
    do
    {
        *--ptr = (char)('0' + value - 10 * (t = value / 10));
    } while ((value = t) != 0);

    return ptr;
}

std::string Int64ToString(int64_t value)
{
    char buf[32];
    buf[31] = '\0';

    if (value >= 0)
        return std::string(LLToStr(value, buf + 31));
    else
        return "-" + std::string(LLToStr(-value, buf + 31));
}


}
