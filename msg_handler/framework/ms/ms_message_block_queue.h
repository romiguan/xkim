#ifndef __MS_MESSAGE_BLOCK_QUEUE_H__
#define __MS_MESSAGE_BLOCK_QUEUE_H__

#include <pthread.h>

namespace ms
{

struct IMessage;
class MsMessageBlockQueue
{
    public:
        MsMessageBlockQueue();
        ~MsMessageBlockQueue();

        int getQueueSize();
        IMessage* get();
        IMessage* get(int count);

        /*
         * 放入一个msg
         */
        void put(IMessage* msg);

        /*
         * 放入一个msg列表,确保最后的_next为0
         */
        void putList(IMessage* msg);

    private:
        IMessage* _list;
        IMessage* _tail;
        int _list_size;

        pthread_mutex_t _mutex;
        pthread_cond_t _cond;
};

}

#endif
