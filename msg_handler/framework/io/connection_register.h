#ifndef __CONNECTION_REGISTER_H__
#define __CONNECTION_REGISTER_H__

#include <boost/unordered_map.hpp>

namespace io
{

template <typename T>
class ConnectionRegister
{
    public:
        ConnectionRegister() {}
        ~ConnectionRegister() {}

        T* get(uint64_t key)
        {
            typename _MAP::iterator it = _map.find(key);
            return it != _map.end() ? it->second : 0;
        }

        void set(uint64_t key, T* conn)
        {
            _map[key] = conn;
        }

        void unset(uint64_t key)
        {
            _map.erase(key);
        }

    private:
        typedef boost::unordered_map<uint64_t, T*> _MAP;
        _MAP _map;
};

}

#endif
