#include "ms_message_queue.h"
#include "ms_message.h"

#include <stdlib.h>

#include <glog/logging.h>

namespace ms
{

MsMessageQueue::MsMessageQueue():
    _head(0),
    _tail(0)
{
    if (pthread_mutex_init(&_mutex, 0) != 0)
    {
        LOG(ERROR) << "pthread_mutex_init error, abort\n";
        abort();
    }
}

MsMessageQueue::~MsMessageQueue()
{
    pthread_mutex_destroy(&_mutex);
}

IMessage* MsMessageQueue::get()
{
    pthread_mutex_lock(&_mutex);
    IMessage* v = _head;
    _head = _tail = 0;
    pthread_mutex_unlock(&_mutex);
    return v;
}

void MsMessageQueue::put(IMessage* v)
{
    pthread_mutex_lock(&_mutex);
    if (_tail)
        _tail->_next = v;
    else
        _head = v;
    while (v->_next)
        v = v->_next;
    _tail = v;
    pthread_mutex_unlock(&_mutex);
}

}
