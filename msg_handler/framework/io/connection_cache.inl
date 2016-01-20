#include <new>
#include <glog/logging.h>

namespace io
{

template <typename T>
void ConnectionCache<T>::open()
{
    for (int i = 0; i < 1024; ++i)
    {
        ConnectionCache::ConnNode* node = new (std::nothrow) ConnectionCache::ConnNode;
        if (node == 0)
        {
            LOG(ERROR) << "ConnectionCache::open error, abort\n";
            abort();
        }
        _idle_node.add(*node);
    }
}

template <typename T>
void ConnectionCache<T>::recycle(T* conn)
{
    ConnectionCache::ConnNode* node = _idle_node.first();
    if (node)
    {
        LinkType::remove(*node);
    }
    else
    {
        node = new (std::nothrow) ConnectionCache::ConnNode;
        if (node == 0)
        {
            LOG(ERROR) << "memory error, abort\n";
            abort();
        }
    }

    node->_conn = conn;
    node->_which_loop = _loop_count;

    _recycle.addLast(*node);
    LOG(INFO) << "recycle in io(" << _owner <<"), loop count:" << node->_which_loop << std::endl;
}

template <typename T>
T* ConnectionCache<T>::get()
{
    ConnectionCache::ConnNode* node = _cache.first();
    if (node)
    {
        LinkType::remove(*node);
        T* conn = node->_conn;
        node->_conn = 0;
        _idle_node.add(*node);
        LOG(INFO) << "get connection from cache in io(" << _owner <<"), org:" << node->_which_loop << std::endl;
        return conn;
    }
    else
    {
        bool flag = false;
        while (true)
        {
            node = _recycle.first();
            if (node == 0 || _loop_count <= node->_which_loop)
                break;
            flag = true;
            LinkType::remove(*node);
            _cache.addLast(*node);
        }

        T* conn = 0;
        if (flag)
        {
            node = _cache.first();
            LinkType::remove(*node);
            conn = node->_conn;
            node->_conn = 0;
            _idle_node.add(*node);
        }
        else
        {
            conn = new (std::nothrow) T();
            if (conn == 0)
            {
                LOG(ERROR) << "memory error, abort\n";
                abort();
            }
        }

        return conn;
    }
}

}
