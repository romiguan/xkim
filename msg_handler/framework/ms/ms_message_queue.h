#ifndef __MS_MESSAGE_QUEUE_H__
#define __MS_MESSAGE_QUEUE_H__

#include <pthread.h>

namespace ms
{

class IMessage;
class MsMessageQueue
{
    public:
        MsMessageQueue();
        ~MsMessageQueue();

        IMessage* get();
        void put(IMessage* msg);

    private:
        pthread_mutex_t _mutex;
        IMessage* _head;
        IMessage* _tail;
};

}

#endif
