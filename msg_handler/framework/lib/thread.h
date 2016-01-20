#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

namespace util
{

class Thread;

/**
 * the core work belong to Thread
 */
class Runnable
{
    public:
        Runnable() {}
        virtual ~Runnable() {}

        /**
         * execute the real work
         */
        virtual int run() = 0;

        /**
         * stop the real work
         */ 
        virtual int stop() { return 0; }

        /**
         * get associated thread
         */
        boost::shared_ptr<Thread> thread() { return _binded_thread.lock(); }

        /**
         * set associated thread, bind the thread
         *
         * @param this_thread : the associated thread
         */
        void thread(boost::shared_ptr<Thread> this_thread) { _binded_thread = this_thread; }

    private:
        boost::weak_ptr<Thread> _binded_thread;
};

/**
 * abstract class for thread
 */
class Thread
{
    public:
        typedef pthread_t t_id;

        Thread() {}
        virtual ~Thread() {}

        /**
         * start this thread
         */
        virtual int run() = 0;

        /**
         * stop this thread
         */
        virtual int stop() = 0;

        /**
         * join this thread
         */
        virtual int join() = 0;

        /**
         * get thread id by Thread Object
         */
        virtual t_id getThreadid() = 0;

#ifdef CPU_AFFINITY
        /**
         * set cpu affinity 
         */
        virtual void setCpu(const std::vector<int> &cpu_set)
        {
            (void)cpu_set;
        }
#endif
        boost::shared_ptr<Runnable> runnable() { return _binded_runnable; }

    protected:
        /**
         * just call this fun when creating concrete thread, after that, can't modify the runnable
         */
        void runnable(boost::shared_ptr<Runnable> this_runnable) { _binded_runnable = this_runnable; }

    public:
        static inline t_id currentTid() { return pthread_self(); }
        static inline bool isCurrent(t_id tid) { return pthread_equal(pthread_self(), tid); }

    protected:
#ifdef CPU_AFFINITY
        std::vector<int> _cpu_set;
#endif
    private:
        boost::shared_ptr<Runnable> _binded_runnable;
};

class ThreadFactory
{
    public:
        ThreadFactory() {}
        virtual ~ThreadFactory() {}

        virtual boost::shared_ptr<Thread> newThread(boost::shared_ptr<Runnable> runnable) const = 0;
};

}

#endif
