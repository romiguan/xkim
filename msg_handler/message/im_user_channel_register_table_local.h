#ifndef __IM_USER_CHANNEL_REGISTER_TABLE_LOCAL_H__
#define __IM_USER_CHANNEL_REGISTER_TABLE_LOCAL_H__

#include "framework/io/io_channel.h"

#include <pthread.h>

#include <boost/unordered_map.hpp>

namespace im
{

class ImUserChannelRegisterTableLocal
{
    public:
        ImUserChannelRegisterTableLocal();
        ~ImUserChannelRegisterTableLocal();

        void get(UID key, io::IoChannel& channel)
        {
            channel._uid = -1;
            pthread_mutex_lock(&_mutex);
            _MAP::iterator it = _map.find(key);
            if (it != _map.end())
                channel = it->second;
            pthread_mutex_unlock(&_mutex);
        }

        void set(UID key, io::IoChannel& channel)
        {
            pthread_mutex_lock(&_mutex);
            _map[key] = channel;
            pthread_mutex_unlock(&_mutex);
        }

        void unset(UID key)
        {
            pthread_mutex_lock(&_mutex);
            _map.erase(key);
            pthread_mutex_unlock(&_mutex);
        }

        void getOrUnset(UID key, uint64_t conn, io::IoChannel& channel)
        {
            channel._uid = -1;
            pthread_mutex_lock(&_mutex);
            _MAP::iterator it = _map.find(key);
            if (it != _map.end())
            {
                if (conn >= it->second._conn)
                    _map.erase(key);
                else
                    channel = it->second;
            }
            pthread_mutex_unlock(&_mutex);
        }

    private:
        typedef boost::unordered_map<UID, io::IoChannel> _MAP;
        _MAP _map;
        pthread_mutex_t _mutex;
};

}

#endif
