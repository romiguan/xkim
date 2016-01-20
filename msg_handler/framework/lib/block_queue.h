#ifndef __BLOCK_QUEUE_H__
#define __BLOCK_QUEUE_H__

#include <pthread.h>

namespace util
{

class BlockQueue
{
    public:
        BlockQueue();
        ~BlockQueue();

        void open();

        void* get();
        int getM(void** v, int want);

        void put(void* v);

    private:
        void* _get();
        void _put(void* v);

    private:
        static const int DEFAULT_NODE_COUNT = 1024;

    private:
        struct InternalNode
        {
            InternalNode* _next;
            void* _v;
        };

    private:
        InternalNode* _idle_list;
        InternalNode* _list;
        InternalNode* _tail;
        int _list_size;

        pthread_mutex_t _mutex;
        pthread_cond_t _cond;
};

}

#endif
