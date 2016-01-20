#ifndef __MS_MESSAGE_PRIORITY_QUEUE_H__
#define __MS_MESSAGE_PRIORITY_QUEUE_H__

#include "ms_message_unsafe_queue.h"

#include <pthread.h>

namespace ms
{

class IMessage;
class MsMessagePriorityQueue
{
    public:
        MsMessagePriorityQueue();
        ~MsMessagePriorityQueue();

        void init(int priority_num);
        int total_size();
        IMessage* get();
        //IMessage* get(int count);

        void put(IMessage* msg, int priority);
        //void putList(IMessage* msg);

    private:
        IMessage* _get();

    private:
        int _priority_num;
        MsMessageUnsafeQueue* _pri_queue;
        int _total_size;

        pthread_mutex_t _mutex;
        pthread_cond_t _cond;
};

}

#endif
