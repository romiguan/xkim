#ifndef __POSIX_THREAD_FACTORY_H__
#define __POSIX_THREAD_FACTORY_H__

#include "thread.h"

namespace util
{

class PosixThreadFactory : public ThreadFactory
{
    public:
        enum POLICY
        {
            OTHER,
            FIFO,
            ROUND_ROBIN
        };

        enum PRIORITY
        {
            LOWEST = 0,
            LOWER = 1,
            LOW = 2,
            NORMAL = 3,
            HIGH = 4,
            HIGHER = 5,
            HIGHEST = 6,
        };

    public:
        PosixThreadFactory(POLICY policy=ROUND_ROBIN, PRIORITY priority=NORMAL, int stackSize=10, bool detached=true):
            _thread_policy(policy),
            _thread_priority(priority),
            _thread_stack_size(stackSize),
            _thread_detached(detached)
        {
        }

        virtual ~PosixThreadFactory() {}

        void setStackSize(int stack_size /*MB*/) { _thread_stack_size = stack_size; }
        int getStackSize() { return _thread_stack_size; }

        void setDetached(bool detached) { _thread_detached = detached; }
        bool isDetached() const { return _thread_detached; }

        void setPolicy(POLICY policy) { _thread_policy = policy; }
        POLICY getPolicy() const { return _thread_policy; }

        void setPriority(PRIORITY priority) { _thread_priority = priority; }
        PRIORITY getPriority() const { return _thread_priority; }

        //from ThreadFactory
        virtual boost::shared_ptr<Thread> newThread(boost::shared_ptr<Runnable> runnable) const;

    private:
        int toPthreadPolicy(POLICY policy) const
        {
            switch (policy)
            {
                case OTHER:
                    return SCHED_OTHER;
                case FIFO:
                    return SCHED_FIFO;
                case ROUND_ROBIN:
                    return SCHED_RR;
                default:
                    return SCHED_OTHER;
            }
        }

        int toPthreadPriority(PRIORITY priority, POLICY policy) const
        {
            int pthread_policy = toPthreadPolicy(policy);
            int min_priority = 0;
            int max_priority = 0;

            min_priority = sched_get_priority_min(pthread_policy);
            max_priority = sched_get_priority_max(pthread_policy);

            int delta = (HIGHEST - LOWEST) + 1;
            float step = (float)(max_priority - min_priority) / delta;

            if (priority > HIGHEST)
                priority = NORMAL;

            return (int)(min_priority + step * priority);
        }

    private:
        POLICY _thread_policy;
        PRIORITY _thread_priority;
        int _thread_stack_size;
        bool _thread_detached;
};

}

#endif
