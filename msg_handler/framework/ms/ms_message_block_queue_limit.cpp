#include "ms_message_block_queue_limit.h"

#include <stdlib.h>
#include <stdio.h>

#include <glog/logging.h>

namespace io
{

MsMessageBlockQueueLimit::MsMessageBlockQueueLimit():
    _list(0),
    _tail(0),
    _list_size(0),
    _block_threshold(10000),
    _unblock_threshold(5000),
    _flag(NORMAL)
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

MsMessageBlockQueueLimit::~MsMessageBlockQueueLimit()
{
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
}

ms::IMessage* MsMessageBlockQueueLimit::get()
{
    pthread_mutex_lock(&_mutex);
    while (_list_size == 0)
        pthread_cond_wait(&_cond, &_mutex);
    ms::IMessage* value = _list;
    _list = _tail = 0;
    _list_size = 0;
    pthread_mutex_unlock(&_mutex);

    return value;
}

ms::IMessage* MsMessageBlockQueueLimit::get(int count)
{
    pthread_mutex_lock(&_mutex);
    while (_list_size == 0)
        pthread_cond_wait(&_cond, &_mutex);

    ms::IMessage* v = _list;
    if (_list_size > count)
    {
        int size = count;
        ms::IMessage* item = _list;
        while (--count)
            item = item->_next;
        _list = item->_next;
        item->_next = 0;
        _list_size -= size;
    }
    else
    {
        _list = _tail = 0;
        _list_size = 0;
    }
    pthread_mutex_unlock(&_mutex);

    return v;
}

int MsMessageBlockQueueLimit::put(ms::IMessage* v)
{
    int ret = 0;
    v->_next = 0;
    pthread_mutex_lock(&_mutex);
    switch (_flag)
    {
        case NORMAL:
        {
            if (_list_size < _block_threshold)
            {
                _put(v);
                ret = 0;
            }
            else
            {
                _flag = BLOCK;
                ret = -1;
            }
        }
        case BLOCK:
        {
            if (_list_size <= _unblock_threshold)
            {
                _flag = NORMAL;
                _put(v);
                ret = 0;
            }
            else
            {
                ret = -1;
            }
        }
        default:
        {
            abort();
        }
    }
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
    return ret;
}

void MsMessageBlockQueueLimit::_put(ms::IMessage* v)
{
    if (_tail)
        _tail->_next = v;
    else
        _list = v;
    _tail = v;
    ++_list_size;
}

}
