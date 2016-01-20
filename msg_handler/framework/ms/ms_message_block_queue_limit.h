#ifndef __MS_MESSAGE_BLOCK_QUEUE_LIMIT_H__
#define __MS_MESSAGE_BLOCK_QUEUE_LIMIT_H__

#include "ms/ms_message.h"

#include <pthread.h>

namespace io
{

struct IMessage;
class MsMessageBlockQueueLimit
{
    public:
        MsMessageBlockQueueLimit();
        ~MsMessageBlockQueueLimit();

        ms::IMessage* get();
        ms::IMessage* get(int count);

        /*
         * 放入一个msg
         */
        int put(ms::IMessage* v);

    private:
        void _put(ms::IMessage* v);

    private:
        enum
        {
            NORMAL = 1,
            BLOCK,
        };

    private:
        ms::IMessage* _list;
        ms::IMessage* _tail;
        int _list_size;
        int _block_threshold;
        int _unblock_threshold;
        int _flag;

        pthread_mutex_t _mutex;
        pthread_cond_t _cond;
};

}

#endif
