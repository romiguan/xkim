#include "ms_message_unsafe_queue.h"
#include "ms_message.h"

#include <stdlib.h>
#include <stdio.h>

namespace ms
{

MsMessageUnsafeQueue::MsMessageUnsafeQueue():
    _head(0),
    _tail(0),
    _total_size(0)
{
}

MsMessageUnsafeQueue::~MsMessageUnsafeQueue()
{
}

IMessage* MsMessageUnsafeQueue::getAll()
{
    IMessage* msg = _head;
    _head = _tail = 0;
    _total_size = 0;
    return msg;
}

IMessage* MsMessageUnsafeQueue::get()
{
    IMessage* msg = _head;
    if (_head)
    {
        _head = _head->_next;
        if (_head == 0)
            _tail = 0;
        --_total_size;
    }
    return msg;
}

void MsMessageUnsafeQueue::put(IMessage* msg)
{
    if (_tail)
        _tail->_next = msg;
    else
        _head = msg;
    _tail = msg;
    ++_total_size;
}

}
