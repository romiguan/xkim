#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>
#include <stdint.h>

namespace util
{

class Queue
{
    public:
        struct QueueElement
        {
            QueueElement* _next;
            uint64_t _id;
            int _value;
        };

    public:
        Queue();
        ~Queue();

        Queue::QueueElement* get();
        void put(int v, uint64_t id);

    public:
        static void freeQueueElement(QueueElement* v);

    private:
        pthread_mutex_t _mutex;
        QueueElement* _head;
        QueueElement* _tail;
};

}

#endif
