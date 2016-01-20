#include "simple_queue.h"

#include <stdlib.h>
#include <stdio.h>

#include <glog/logging.h>

namespace util
{

SimpleQueue::SimpleQueue():
    _idle_list(0),
    _list(0),
    _tail(0),
    _list_size(0)
{
}

SimpleQueue::~SimpleQueue()
{
}

void SimpleQueue::open()
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

void* SimpleQueue::get()
{
    void* value = 0;
    if (_list_size > 0)
    {
        value = _get();
    }

    return value;
}


int SimpleQueue::getM(void** v, int want)
{
    int c = 0;
    InternalNode* t = 0;
    if (_list_size <= 0)
        return 0;
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

    return c;
}

void SimpleQueue::put(void* v)
{
    _put(v);
}

void* SimpleQueue::_get()
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

void SimpleQueue::_put(void* v)
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
