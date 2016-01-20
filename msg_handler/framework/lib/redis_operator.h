#ifndef _REDIS_OPERATER_H_
#define _REDIS_OPERATER_H_
#include "hiredis/hiredis.h"
#include <string>
#include <vector>

namespace redisop {

class RedisReplyObj
{
public:
    RedisReplyObj(redisReply* reply)
        : m_reply(reply)
    { }

    ~RedisReplyObj()
    { freeReplyObject(m_reply); }

private:
    redisReply* m_reply;
};

class RedisOperator
{
public:
    RedisOperator(const std::string& host, int port);
    RedisOperator();
    ~RedisOperator();

    void SetHostPort(const std::string& host, int port)
    {
        m_redis_host = host;
        m_redis_port = port;
    }

    void SetConnTimeout(const struct timeval& tv)
    {
        m_conn_timeout.tv_sec = tv.tv_sec;
        m_conn_timeout.tv_usec = tv.tv_usec;
    }

    void SetIoTimeout(const struct timeval& tv)
    {
        m_io_timeout.tv_sec = tv.tv_sec;
        m_io_timeout.tv_usec = tv.tv_usec;
    }

    void SetChannel(const std::string& channel) { m_channel = channel; }
    void SetDbIndex(int idx) { m_redis_db_index = idx; }
    void SetKeysNumberPerMGET(int nbr) { m_keys_nbr_per_mget = nbr; }

    bool Connect();
    void FreeRedisConnection();
    redisContext*& GetRedisCursor() { return m_redis;}
    bool GetRedisConnection();
    bool SetRedisDB();

    bool Set(const std::string& key, const std::string& value);
    bool MSet(const std::vector<std::pair<std::string, std::string> >& pairs);
    bool Get(const std::string& key, std::string& value);
    bool MGet(const std::vector<std::string>& keys, std::vector<std::string>& values);
    bool Pub(const std::string& msg);
    bool Keys(std::vector<std::string>& keys);

private:
    bool _CheckReply(const redisReply* reply);
    void _SetDefaultTimeout();

private:
    static const int DEFAULT_CONN_TIMEOUT_SEC = 0;
    static const int DEFAULT_CONN_TIMEOUT_USEC = 20000; // 20ms
    static const int DEFAULT_IO_TIMEOUT_SEC = 0;
    static const int DEFAULT_IO_TIMEOUT_USEC = 10000; // 10ms
    static const int KEYS_NUMBER_PER_MGET = 100;

    std::string m_redis_host;
    int m_redis_port;
    int m_redis_db_index;	
    int m_keys_nbr_per_mget;
    struct timeval m_conn_timeout;
    struct timeval m_io_timeout;
    redisContext *m_redis;
    std::string m_channel;
};
} //namespace redisop
#endif
