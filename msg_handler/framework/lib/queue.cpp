#include "queue.h"

#include <stdlib.h>

#include <glog/logging.h>

namespace util
{

Queue::Queue():
    _head(0),
    _tail(0)
{
    if (pthread_mutex_init(&_mutex, 0) != 0)
    {
        LOG(ERROR) << "pthread_mutex_init error, abort\n";
        abort();
    }
}

Queue::~Queue()
{
    pthread_mutex_destroy(&_mutex);
}

void Queue::put(int v, uint64_t id)
{
    QueueElement* e = (QueueElement*)malloc(sizeof(QueueElement));
    if (e == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    e->_next = 0;
    e->_id = id;
    e->_value = v;

    pthread_mutex_lock(&_mutex);
    if (_tail)
        _tail->_next = e;
    else
        _head = e;
    _tail = e;
    pthread_mutex_unlock(&_mutex);
}

Queue::QueueElement* Queue::get()
{
    pthread_mutex_lock(&_mutex);
    QueueElement* e = _head;
    _head = _tail = 0;
    pthread_mutex_unlock(&_mutex);
    return e;
}

void Queue::freeQueueElement(QueueElement* v)
{
    free(v);
}

}
