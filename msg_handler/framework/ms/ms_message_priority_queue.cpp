#include "ms_message_priority_queue.h"
#include "ms_message.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <glog/logging.h>

namespace ms
{

MsMessagePriorityQueue::MsMessagePriorityQueue():
    _pri_queue(0),
    _total_size(0)
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

MsMessagePriorityQueue::~MsMessagePriorityQueue()
{
    delete []_pri_queue;
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
}

void MsMessagePriorityQueue::init(int priority_num)
{
    assert(priority_num > 0);

    if (priority_num <= 0)
        priority_num = 1;

    _priority_num = priority_num;
    _pri_queue = new (std::nothrow) MsMessageUnsafeQueue[_priority_num];
    if (_pri_queue == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }
}

int MsMessagePriorityQueue::total_size()
{
    int size = 0;
    for (int i = 0; i < _priority_num; ++i)
    {
        size += _pri_queue[i].size();
    }
    
    return size;
}

IMessage* MsMessagePriorityQueue::get()
{
    IMessage* value = 0;
    pthread_mutex_lock(&_mutex);
    //while (total_size() == 0)
    while (_total_size == 0)
        pthread_cond_wait(&_cond, &_mutex);
    value = _get();
    value->_next = 0;
    --_total_size;
    pthread_mutex_unlock(&_mutex);

    return value;
}

void MsMessagePriorityQueue::put(IMessage* msg, int priority)
{
    if (priority >= _priority_num)
        return;

    pthread_mutex_lock(&_mutex);
    _pri_queue[priority].put(msg);
    ++_total_size;
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
}

IMessage* MsMessagePriorityQueue::_get()
{
    for (int i = 0; i < _priority_num; ++i)
    {
        MsMessageUnsafeQueue& q = _pri_queue[i];
        if (q.size() > 0)
            return q.get();
    }
    return 0;
}

}
