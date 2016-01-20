#include "block_queue.h"

#include <stdlib.h>
#include <stdio.h>

#include <glog/logging.h>

namespace util
{

BlockQueue::BlockQueue():
    _idle_list(0),
    _list(0),
    _tail(0),
    _list_size(0)
{
    if (pthread_mutex_init(&_mutex, 0) != 0)
    {
        LOG(ERROR) << "pthread_mutex_init error, abort\n";
        abort();
    }

    if (pthread_cond_init(&_cond, 0) != 0)
    {
        LOG(ERROR) << "pthread_cond_init error, abort\n";
        abort();
    }
}

BlockQueue::~BlockQueue()
{
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
}

void BlockQueue::open()
{
    InternalNode* node = 0;
    for (int i = 0; i < DEFAULT_NODE_COUNT; ++i)
    {
        node = new (std::nothrow) InternalNode;
        if (node == 0)
        {
            LOG(ERROR) << "memory error, abort\n";
            abort();
        }
        node->_next = _idle_list;
        _idle_list = node;
    }
}

void* BlockQueue::get()
{
    void* value = 0;
    pthread_mutex_lock(&_mutex);
    while (_list_size == 0)
        pthread_cond_wait(&_cond, &_mutex);
    value = _get();
    pthread_mutex_unlock(&_mutex);

    return value;
}


int BlockQueue::getM(void** v, int want)
{
    int c = 0;
    InternalNode* t = 0;
    pthread_mutex_lock(&_mutex);
    while (_list_size == 0)
        pthread_cond_wait(&_cond, &_mutex);
    c = want > _list_size ? _list_size : want;
    InternalNode* node = _list;
    for (int i = 0; i < c; ++i)
    {
        t = node;
        v[i] = node->_v;
        node = node->_next;

        t->_next = _idle_list;
        _idle_list = t;
    }
    _list = node;
    if (_list == 0)
        _tail = 0;
    _list_size -= c;
    pthread_mutex_unlock(&_mutex);

    return c;
}

void BlockQueue::put(void* v)
{
    pthread_mutex_lock(&_mutex);
    _put(v);
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
}

void* BlockQueue::_get()
{
    InternalNode* node = _list;
    _list = _list->_next;
    if (_list == 0)
        _tail = 0;
    node->_next = _idle_list;
    _idle_list = node;
    --_list_size;

    return node->_v;
}

void BlockQueue::_put(void* v)
{
    InternalNode* node = _idle_list;
    if (node)
    {
        _idle_list = _idle_list->_next;
    }
    else
    {
        node = new (std::nothrow) InternalNode;
        if (node == 0)
        {
            LOG(ERROR) << "memory error, abort\n";
            abort();
        }
    }

    node->_v = v;
    node->_next = 0;

    if (_tail)
        _tail->_next = node;
    else
        _list = node;
    _tail = node;

    ++_list_size;
}

}
