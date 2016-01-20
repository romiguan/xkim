#ifndef __ROUTE_INFO_PUBLISHER_H__
#define __ROUTE_INFO_PUBLISHER_H__

#include <string>
#include <sys/time.h> // for timeval
#include "framework/io/io_channel.h"
#include "framework/lib/redis_operator.h"
#include "framework/lib/TLS.h"

namespace im {

class BoolWrapper{
public:
    BoolWrapper()
        :_bool_value(false)
    { }
    ~BoolWrapper() {}
    bool get() {return _bool_value;}
    void set(bool val)
    { _bool_value = val; }
private:
    bool _bool_value;
};

class RouteInfoPublisher
{
public:
    RouteInfoPublisher() {}
    ~RouteInfoPublisher();

    bool init(const std::string& host,
              int port,
              const struct timeval& conn_tv, 
              const struct timeval& io_tv,
              const std::string& channel,
              int db_index);
    bool publishRouteInfo(UID uid, uint64_t msgh_id);

private:
    std::string    _host;
    int            _port;
    struct timeval _conn_tv;
    struct timeval _io_tv;
    std::string    _channel;
    int            _db_index;
    util::TLS<BoolWrapper> _init_done;
    util::TLS<redisop::RedisOperator> _redis_op;
};

} //namespace im
#endif //__ROUTE_INFO_PUBLISHER_H__
