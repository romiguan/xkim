#ifndef __IO_PUSH_LIST_H__
#define __IO_PUSH_LIST_H__

#include "ms/ms_message.h"

#include <stdlib.h>

#include <glog/logging.h>

namespace io
{

struct IoPushData
{
    struct IoPushData* _next;
    ms::IMessage* _msg;
    void (*_freev)(void* v);

    char* _wbuf;
    int _wsize;
    int _wpos;
};

inline IoPushData* createPushData();
inline void destroyPushData(IoPushData* data);
inline void destroyMsgData(IoPushData* data);

class IoPushList 
{
    public:
        IoPushList():_list(0),_tail(0),_list_size(0) {}
        ~IoPushList() {}

        IoPushData* get()
        {
            IoPushData* item = _list;
            if (item)
            {
                _list = _list->_next;
                item->_next = 0;
                item->_msg->_next = 0;

                if (_list == 0)
                    _tail = 0;

                --_list_size;
            }
            return item;
        }

        IoPushData* getAll()
        {
            IoPushData* item = _list;
            _list = _tail = 0;
            _list_size = 0;
            return item;
        }

        void put(IoPushData* d)
        {
            d->_msg->_next = 0;
            d->_next = 0;
            if (_tail)
            {
                _tail->_next = d;
                _tail->_msg->_next = d->_msg;
            }
            else
            {
                _list = d;
            }
            _tail = d;
            ++_list_size;
        }

        void putFirst(IoPushData* d)
        {
            d->_msg->_next = 0;
            d->_next = _list;
            if (_tail == 0)
                _tail = d;
            else
                d->_msg->_next = _list->_msg;
            _list = d;
            ++_list_size;
        }

        void clear()
        {
            IoPushData* item;
            while (_list)
            {
                item = _list;
                _list = _list->_next;

                destroyMsgData(item);
            }

            _list = _tail = 0;
            _list_size = 0;
        }

        void shallowClear()
        {
            IoPushData* item;
            while (_list)
            {
                item = _list;
                _list = _list->_next;

                destroyPushData(item);
            }

            _list = _tail = 0;
            _list_size = 0;
        }

        ms::IMessage* stealMsg()
        {
            return _list ? _list->_msg : 0;
        }

        int size() const { return _list_size; }

        bool empty() const { return _list == _tail; }

    private:
        IoPushData* _list;
        IoPushData* _tail;
        int _list_size;
};

typedef IoPushList IoPushBuffer;

inline IoPushData* createPushData()
{
    IoPushData* data = (IoPushData*)malloc(sizeof(IoPushData));
    if (data == 0)
    {
        LOG(ERROR) << "emeory error, abort\n";
        abort();
    }
    return data;
}

inline void destroyPushData(IoPushData* data)
{
    free(data);
}

inline void destroyMsgData(IoPushData* data)
{
    data->_freev(data->_msg);
    destroyPushData(data);
}

}

#endif
