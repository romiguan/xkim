#include "priority_queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h> // for assert()

#include <glog/logging.h>

namespace util
{

PriorityQueue::PriorityQueue(int priority_num):
    _priority_num(priority_num),
    _pri_queue(NULL)
{
    if (_priority_num <= 0)
        _priority_num = 1;

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

PriorityQueue::~PriorityQueue()
{
    delete []_pri_queue;
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
}

void PriorityQueue::init()
{
    assert(_priority_num > 0);
    _pri_queue = new (std::nothrow) SimpleQueue[_priority_num];

    if (_pri_queue == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    for (int i = 0; i < _priority_num; ++i)
        _pri_queue[i].open();
}

int PriorityQueue::total_size()
{
    assert(_priority_num > 0);
    int size = 0;
    for (int i = 0; i < _priority_num; ++i)
    {
        size += _pri_queue[i].size();
    }
    
    return size;
}

void* PriorityQueue::get()
{
    void* value = 0;
    pthread_mutex_lock(&_mutex);
    while (total_size() == 0)
        pthread_cond_wait(&_cond, &_mutex);
    value = _get();
    pthread_mutex_unlock(&_mutex);

    return value;
}

//
// getM, the return result must be IN SEQUENCE
// with priority from HIGH PRIORITY to LOW PRIORITY
//
int PriorityQueue::getM(void** v, int want)
{
    int c = 0;

    pthread_mutex_lock(&_mutex);
    int size = total_size();
    while (size == 0)
        pthread_cond_wait(&_cond, &_mutex);
    c = want > size ? size : want;
    int cur = 0;
    for (int i = 0; i < _priority_num; ++i)
    {
        SimpleQueue& q = _pri_queue[i];
        int ret = q.getM(v+cur, c-cur); 
        if (ret == c - cur)
            break;
        cur += ret;
    }
    pthread_mutex_unlock(&_mutex);

    return c;
}

void PriorityQueue::put(void* v, int priority)
{
    pthread_mutex_lock(&_mutex);
    _put(v, priority);
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
}

void* PriorityQueue::_get()
{
    for (int i = 0; i < _priority_num; ++i)
    {
        SimpleQueue& q = _pri_queue[i];
        if (q.size() > 0)
            return q.get();
    }
    return 0;
}

// priority start from 0
void PriorityQueue::_put(void* v, int priority)
{
    if (priority >= _priority_num)
        return;
    SimpleQueue& q = _pri_queue[priority];
    q.put(v);
}

}
