#ifndef __SIMPLE_QUEUE_H__
#define __SIMPLE_QUEUE_H__

#include <pthread.h>

namespace util
{

//
// Simple Queue, no lock
//
class SimpleQueue
{
    public:
        SimpleQueue();
        ~SimpleQueue();

        void open();

        void* get();
        int getM(void** v, int want);
        void put(void* v);

        int size() { return _list_size; }

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
};

}

#endif // __SIMPLE_QUEUE_H__
