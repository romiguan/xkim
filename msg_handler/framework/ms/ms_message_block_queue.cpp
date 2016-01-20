#include "ms_message_block_queue.h"
#include "ms_message.h"

#include <stdlib.h>
#include <stdio.h>

#include <glog/logging.h>

namespace ms
{

MsMessageBlockQueue::MsMessageBlockQueue():
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

MsMessageBlockQueue::~MsMessageBlockQueue()
{
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
}

int MsMessageBlockQueue::getQueueSize()
{
    int size = 0;
    pthread_mutex_lock(&_mutex);
    size = _list_size;
    pthread_mutex_unlock(&_mutex);
    return size;
}

ms::IMessage* MsMessageBlockQueue::get()
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

ms::IMessage* MsMessageBlockQueue::get(int count)
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

void MsMessageBlockQueue::put(ms::IMessage* v)
{
    v->_next = 0;
    pthread_mutex_lock(&_mutex);
    if (_tail)
        _tail->_next = v;
    else
        _list = v;
    _tail = v;
    ++_list_size;
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
}

void MsMessageBlockQueue::putList(ms::IMessage* v)
{
    pthread_mutex_lock(&_mutex);
    if (_tail)
        _tail->_next = v;
    else
        _list = v;
    int count = 1;
    while(v->_next)
    {
        v = v->_next;
        ++count;
    }
    _tail = v;
    _list_size += count;;
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
}

}
