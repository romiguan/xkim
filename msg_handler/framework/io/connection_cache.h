#ifndef __CONNECTION_CACHE_H__
#define __CONNECTION_CACHE_H__

#include "lib/linked_list.h"

using util::LinkedListType;

namespace io
{

template <typename T>
class ConnectionCache
{
    public:
        explicit ConnectionCache(int owner):
            _loop_count(0),
            _owner(owner)
        {
        }
        ~ConnectionCache() {}

        void open();

        void recycle(T* conn);
        T* get();

    private:
        struct ConnNode
        {   
            util::ListNodeLinkType _node;
            T* _conn;
            //loop count when recycle
            int64_t _which_loop;
        };

        typedef util::LinkedListType<ConnNode, &ConnNode::_node> LinkType;

    public:
        int64_t _loop_count;

    private:
        int _owner;
        LinkType _idle_node;
        LinkType _cache;
        LinkType _recycle;
};

}

#include "connection_cache.inl"

#endif
