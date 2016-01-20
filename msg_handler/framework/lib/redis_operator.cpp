#include "redis_operator.h"
#include <glog/logging.h>
#include <time.h>
#include <assert.h>

namespace redisop {

//
// redis operator with re-connection when error/kicked
//

void RedisOperator::_SetDefaultTimeout()
{
    m_conn_timeout.tv_usec = DEFAULT_CONN_TIMEOUT_USEC;
    m_conn_timeout.tv_sec = DEFAULT_CONN_TIMEOUT_SEC;

    m_io_timeout.tv_usec = DEFAULT_IO_TIMEOUT_USEC;
    m_io_timeout.tv_sec = DEFAULT_IO_TIMEOUT_SEC;
}

RedisOperator::RedisOperator(const std::string& host, int port)
    : m_redis_host(host), 
      m_redis_port(port), 
      m_redis_db_index(-1), 
      m_keys_nbr_per_mget(KEYS_NUMBER_PER_MGET),
      m_redis(NULL)
{
    _SetDefaultTimeout();
}

RedisOperator::RedisOperator()
   : m_redis_host(""),
     m_redis_port(0), 
     m_redis_db_index(-1), 
     m_keys_nbr_per_mget(KEYS_NUMBER_PER_MGET),
     m_redis(NULL)
{
    _SetDefaultTimeout();
}

RedisOperator::~RedisOperator()
{
    FreeRedisConnection();
}

void RedisOperator::FreeRedisConnection()
{
    if (NULL != m_redis) {
        redisFree(m_redis);
        m_redis = NULL;
    }
}

bool RedisOperator::_CheckReply(const redisReply* reply)
{
    if (reply == NULL || REDIS_REPLY_ERROR == reply->type) {
        LOG(ERROR) << "Redis Reply Error: " << reply->str << "\n";
        return false;
    }
    return true;
}

// if Connect() failed, FreeRedisConnection is called inside this function
// NO NEED to call FreeRedisConnection outside this function
bool RedisOperator::Connect()
{
    if (!GetRedisConnection()) {
        FreeRedisConnection();
        LOG(ERROR) << "Connect to redis failed, connect to Redis server failed.";
        return false;
    }

    if (redisSetTimeout(m_redis, m_io_timeout) != REDIS_OK) {
        LOG(ERROR) << "Set redis IO timeout failed, connect to Redis server failed.";
        FreeRedisConnection();
        return false;
    }

    if (!SetRedisDB()) {
        LOG(ERROR) << "Set redis DB failed, connect to Redis server failed.";
        FreeRedisConnection();
        return false;
    }

    return true;
}

bool RedisOperator::GetRedisConnection()
{
    if (m_redis_host.empty() || m_redis_port == 0) {
        LOG(ERROR) << "Redis host or port is empty, please SetHostPort() first";
        return false;
    }
    m_redis = redisConnectWithTimeout(m_redis_host.c_str(), m_redis_port, m_conn_timeout);
    if (NULL == m_redis || m_redis->err) {
        LOG(ERROR) << "Failed to connect to redis server " << m_redis_host << ":" << m_redis_port << " with error: " << m_redis->errstr;
        return false;
    }
    return true;
}

bool RedisOperator::SetRedisDB()
{
    if (m_redis_db_index == -1) {
        LOG(ERROR) << "Please select redis db first";
        return false;
    }
    // no retry in set redis db
    // consider select db as a mandatory step for connect to redis server
    if (m_redis == NULL) {
        return false; // could NOT happen in real env
    }
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis, "SELECT %d", m_redis_db_index));
    RedisReplyObj robj(reply);
    if (reply == NULL || !_CheckReply(reply)) {
        return false;
    }
    return true;
}

bool RedisOperator::Set(const std::string& key, const std::string& value)
{
    if (m_redis == NULL && !Connect()) {
        LOG(ERROR) << "In SET, connect to redis failed.";
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis, "SET %s %s", key.c_str(), value.c_str()));

    if (reply == NULL && m_redis->err != REDIS_OK) {
        LOG(INFO) << "Reconnect to redis server...";
        FreeRedisConnection();
        if (!Connect()) {
            return false;
        }
        reply = static_cast<redisReply*>(redisCommand(m_redis, "SET %s %s", key.c_str(), value.c_str()));
    } 
    RedisReplyObj robj(reply);
    
    if (reply == NULL || !_CheckReply(reply)) {       
        return false;
    }
    return true;
}

bool RedisOperator::MSet(const std::vector<std::pair<std::string, std::string> >& pairs)
{
    if (m_redis == NULL && !Connect()) {
        LOG(ERROR) << "In MSET, connect to redis failed.";
        return false;
    }

    std::string command = "MSET";
    for(size_t i = 0; i < pairs.size(); ++i) {
        command.append(" ");
        command.append(pairs[i].first);
        command.append(" ");
        command.append(pairs[i].second);
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis, command.c_str()));
    if (reply == NULL && m_redis->err != REDIS_OK) {
        LOG(INFO) << "Reconnect to redis server...";
        FreeRedisConnection();
        if (!Connect()) {
            return false;
        }
        reply = static_cast<redisReply*>(redisCommand(m_redis, command.c_str()));
    } 
    
    RedisReplyObj rojb(reply);
    if (reply == NULL || !_CheckReply(reply)) {       
        return false;
    }
    return true;
}

bool RedisOperator::Get(const std::string& key, std::string& value)
{
    if (m_redis == NULL && !Connect()) {
        LOG(ERROR) << "In MSET, connect to redis failed.";
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis, "GET %s", key.c_str()));
    if (reply == NULL && m_redis->err != REDIS_OK) {
        LOG(INFO) << "Reconnect to redis server...";
        FreeRedisConnection();
        if (!Connect()) {
            return false;
        }
        reply = static_cast<redisReply*>(redisCommand(m_redis,  "GET %s", key.c_str()));
    } 
     
    RedisReplyObj rojb(reply);
    if (reply == NULL || !_CheckReply(reply)) {       
        return false;
    }

    if (reply && reply->type == REDIS_REPLY_STRING && reply->len >= 0 && reply->str) {
        value.assign(reply->str, (size_t)reply->len);
        return true;
    } else {
        return false;
    }
}		

bool RedisOperator::MGet(const std::vector<std::string>& keys, std::vector<std::string>& values)
{
    if (m_redis == NULL && !Connect()) {
        LOG(ERROR) << "In MSET, connect to redis failed.";
        return false;
    }

    size_t key_size = keys.size();
    values.resize(key_size); // must resize
    std::string command = "MGET";
    for (size_t i = 0; i < key_size; i += m_keys_nbr_per_mget) {
        size_t key_size_per_op = 0;
        if (i + m_keys_nbr_per_mget <= key_size) {
            key_size_per_op = m_keys_nbr_per_mget;
        } else {
            key_size_per_op = key_size - i; 
        }
        
        // build MGET command
        std::string command = "MGET";
        for (size_t j = i; j < i + key_size_per_op; ++j) {
            command.append(" ").append(keys[j]); 
        }
        
        // issue MGET command
        redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis, command.c_str()));
        if (reply == NULL && m_redis->err != REDIS_OK) {
            LOG(INFO) << "Reconnect to redis server...";
            FreeRedisConnection();
            if (!Connect()) {
                return false;
            }
            reply = static_cast<redisReply*>(redisCommand(m_redis, command.c_str()));
        } 

        RedisReplyObj robj(reply);
        if (reply == NULL || !_CheckReply(reply)) {       
            std::fill(values.begin()+i, values.begin()+key_size_per_op, "");
            continue;
        }

        if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements >= 0 && reply->element) {
            if (reply->elements != key_size_per_op) {
                // ERROR case, should NOT be happened
                std::fill(values.begin()+i, values.begin()+key_size_per_op, "");
                continue;
            }

            size_t idx = 0;
            while (idx < reply->elements) {
                redisReply* sub_reply = reply->element[idx];
                if (sub_reply && sub_reply->type == REDIS_REPLY_STRING && sub_reply->len >= 0 && sub_reply->str) {
                    values[i+idx] = std::string(sub_reply->str, sub_reply->len);
                } else {
                    values[i+idx] = "";
                }
                idx++;
            }
        } else {
            std::fill(values.begin()+i, values.begin()+key_size_per_op, "");
        }
    }
    return true;
}		

bool RedisOperator::Pub(const std::string& msg)
{
    if (m_redis == NULL && !Connect()) {
        LOG(ERROR) << "In MSET, connect to redis failed.";
        return NULL;
    }

    assert(!m_channel.empty());

    std::string command = "PUBLISH";
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis, "%s %s %s", command.c_str(), m_channel.c_str(),  msg.c_str()));

    if (reply == NULL && m_redis->err != REDIS_OK) {
        LOG(INFO) << "Reconnect to redis server...";
        FreeRedisConnection();
        if (!Connect()) {
            return false;
        }
        reply = static_cast<redisReply*>(redisCommand(m_redis, "%s %s %s", command.c_str(), m_channel.c_str(),  msg.c_str()));
        RedisReplyObj robj(reply);
    } 
    
    if (reply == NULL || !_CheckReply(reply)) {       
        return false;
    } else {
        return true;
    }
}

bool RedisOperator::Keys(std::vector<std::string>& keys)
{
    if (m_redis == NULL && !Connect()) {
        LOG(ERROR) << "In KEYS, connect to redis failed.";
        return false;
    }

    std::string command = "KEYS *";
    // issue KEYS command
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis, command.c_str()));
    if (reply == NULL && m_redis->err != REDIS_OK) {
        LOG(INFO) << "Reconnect to redis server...";
        FreeRedisConnection();
        if (!Connect()) {
            return false;
        }
        reply = static_cast<redisReply*>(redisCommand(m_redis, command.c_str()));
    } 

    RedisReplyObj robj(reply);
    if (reply == NULL || !_CheckReply(reply)) {       
        return false;
    }

    if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements >= 0 && reply->element) {
        size_t idx = 0;
        keys.reserve(reply->elements);
        while (idx < reply->elements) {
            redisReply* sub_reply = reply->element[idx];
            if (sub_reply && sub_reply->type == REDIS_REPLY_STRING && sub_reply->len >= 0 && sub_reply->str) {
                keys.push_back(std::string(sub_reply->str, sub_reply->len));
            }
            idx++;
        }
    }
    return true;
}		
} // namespace redisop
