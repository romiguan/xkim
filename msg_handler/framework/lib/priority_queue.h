#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__

#include <pthread.h>
#include "simple_queue.h" // SimpleQueue

namespace util
{

class PriorityQueue
{
    public:
        PriorityQueue(int priority_num);
        ~PriorityQueue();
        
        void init();
        void* get();
        int getM(void** v, int want);
        void put(void* v, int priority);
        int total_size();

    private:
        void* _get();
        void _put(void* v, int priority);

    private:
        int _priority_num;
        SimpleQueue* _pri_queue;

        pthread_mutex_t _mutex;
        pthread_cond_t _cond;
};

}

#endif //__PRIORITY_QUEUE_H__
