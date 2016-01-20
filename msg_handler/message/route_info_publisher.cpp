#include <glog/logging.h>
#include "route_info_publisher.h"
#include "framework/lib/util.h"

namespace im {
RouteInfoPublisher::~RouteInfoPublisher()
{
}

bool RouteInfoPublisher::init(const std::string& host,
                              int port, 
                              const struct timeval& conn_tv, 
                              const struct timeval& io_tv,
                              const std::string& channel,
                              int db_index)
{
    _host = host;
    _port = port;
    _conn_tv.tv_sec = conn_tv.tv_sec;
    _conn_tv.tv_usec = conn_tv.tv_usec;
    _io_tv.tv_sec = io_tv.tv_sec;
    _io_tv.tv_usec = io_tv.tv_usec;
    _channel = channel;
    _db_index = db_index;

    if (_host.empty())
    {
        LOG(ERROR) << "redis host is empty";
        return false;
    }

    if (_channel.empty())
    {
        LOG(ERROR) << "route info pub channel is empty";
        return false;
    }

    fprintf(stderr, "%s\n", _host.c_str());
    fprintf(stderr, "%d\n", _port);
    fprintf(stderr, "db: %d\n", _db_index);
    fprintf(stderr, "channel: %s\n", _channel.c_str());
    return true; 
}

bool RouteInfoPublisher::publishRouteInfo(UID uid, uint64_t msgh_id)
{
    BoolWrapper* init_done = _init_done.ts_get();//init value is false
    redisop::RedisOperator* redis_op = _redis_op.ts_get();
    if (!init_done->get())
    {
        redis_op->SetHostPort(_host, _port);
        redis_op->SetConnTimeout(_conn_tv);
        redis_op->SetIoTimeout(_io_tv);
        redis_op->SetDbIndex(_db_index);
        redis_op->SetChannel(_channel);
        init_done->set(true);
    }

    std::string uid_str = util::Int64ToString(uid);
    std::string msgh_id_str = util::Uint64ToString(msgh_id);

    //1. set uid->msgh_id to redis DB
    if (!redis_op->Set(uid_str, msgh_id_str))
    {
        LOG(ERROR) << "Set " << uid_str << ":" << msgh_id_str << " to redis failed.";
        return false;
    }

    //2. pub uid->msgh_id to redis DB, format "uid:msgh_id"
    std::string pub_msg = uid_str + ":" + msgh_id_str; 
    if (!redis_op->Pub(pub_msg))
    {
        LOG(ERROR) << "Publish " << pub_msg << " to redis failed.";
        return false;
    }
    return true;
}

}
