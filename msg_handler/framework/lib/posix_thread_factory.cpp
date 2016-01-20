#include "posix_thread_factory.h"

#include <sys/time.h>
#include <sys/types.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

namespace util
{

#define INVALID_THREAD_ID 0

namespace
{

struct ThreadInternal
{
    int fd[2];
    void* thread_arg;
};

class PthreadThread : public Thread
{
    public:
        enum STATE
        {
            UNINIT,
            STARTING,
            STARTED,
            RUNNING,
            STOPPING,
            STOPPED
        };

        static const int MB = 1024*1024;
        static void *thread_fun(void *arg);

    public:
        PthreadThread(int policy, int priority, int stack_size, bool detached, boost::shared_ptr<Runnable> runnable):
            _policy(policy),
            _priority(priority),
            _stack_size(stack_size*MB),
            _detached(detached),
            _bind(false),
            _state(UNINIT)
        {
            assert(_stack_size >= PTHREAD_STACK_MIN);
            assert(runnable != NULL);
            this->Thread::runnable(runnable);
        }

        virtual ~PthreadThread()
        {
            if (_thread_id != INVALID_THREAD_ID)
                join();
        }

        void setDetached(bool detached) { _detached = detached; }
        void setBind(bool bind) { _bind = bind; }

#ifdef CPU_AFFINITY
        /**
         * set cpu affinity 
         */
        void setCpu(const std::vector<int> &cpu_set)
        {
            int cpu_count = sysconf(_SC_NPROCESSORS_CONF);
            for (size_t i = 0; i < cpu_set.size(); ++i)
            {
                if (cpu_set[i] < cpu_count)
                    m_cpu_set.push_back(cpu_set[i]);
            }
        }
#endif

        int run();
        int stop();
        int join();

        Thread::t_id getThreadid()
        {
            return (Thread::t_id)_thread_id;
        }

        void weakRef(boost::shared_ptr<PthreadThread> self)
        {
            assert(this == self.get());
            _self = boost::weak_ptr<PthreadThread>(self);
        }

    private:
        pthread_t _thread_id;
        int _policy;
        int _priority;
        int _stack_size;
        bool _detached;
        bool _bind;
        STATE _state;
        boost::weak_ptr<PthreadThread> _self;
};

int PthreadThread::run()
{
    if (_state != UNINIT)
        return 0;

    pthread_attr_t attr;
    int ret = pthread_attr_init(&attr);
    if (ret != 0)
    {
        fprintf(stderr, "pthread_attr_init: %s\n", strerror(ret));
        return -1;
    }
    ret = pthread_attr_setdetachstate(&attr, _detached ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE);
    if (ret != 0)
    {
        fprintf(stderr, "pthread_attr_setdetachstate: %s\n", strerror(ret));
        return -1;
    }

    pthread_attr_setscope(&attr, _bind ? PTHREAD_SCOPE_SYSTEM : PTHREAD_SCOPE_PROCESS);

    ret = pthread_attr_setstacksize(&attr, _stack_size);
    if (ret != 0)
    {
        fprintf(stderr, "pthread_attr_setstacksize: %s\n", strerror(ret));
        return -1;
    }

    if (_policy != SCHED_OTHER)
    {
        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (ret != 0)
        {
            fprintf(stderr, "pthread_attr_setinheritsched: %s\n", strerror(ret));
            return -1;
        }

        ret = pthread_attr_setschedpolicy(&attr, _policy);
        if (ret != 0)
        {
            fprintf(stderr, "pthread_attr_setschedpolicy: %s\n", strerror(ret));
            return -1;
        }

        struct sched_param param;
        memset(&param, 0, sizeof(param));
        param.sched_priority = _priority;
        //fprintf(stderr, "priority: %d\n", _priority);
        ret = pthread_attr_setschedparam(&attr, &param);
        if (ret != 0)
        {
            fprintf(stderr, "pthread_attr_setschedparam: %s\n", strerror(ret));
            return -1;
        }
    }

    _state = STARTING;
    boost::shared_ptr<PthreadThread>* self = new boost::shared_ptr<PthreadThread>(_self.lock());

    ThreadInternal tharg;
    ret = pipe(tharg.fd);
    assert("create pipe for thread internal" && ret == 0);
    tharg.thread_arg = self;

    ret = pthread_create(&_thread_id, &attr, thread_fun, (void *)&tharg);
    if (ret != 0)
    {
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));
        _state = UNINIT;
        ::close(tharg.fd[0]);
        ::close(tharg.fd[1]);
        return -1;
    }

    char b;
    ret = ::read(tharg.fd[0], &b, 1);
    ::close(tharg.fd[0]);
    ::close(tharg.fd[1]);

    //wait 20ms
    struct timeval tv = {0, 20000};
    select(0, NULL, NULL, NULL, &tv);

    pthread_attr_destroy(&attr);

    return 0;
}

int PthreadThread::stop()
{
    //just notify thread quit, do not block
    if (_state != UNINIT)
        return runnable()->stop();

    return 0;
}

int PthreadThread::join()
{
    if (!_detached && _state != UNINIT)
    {
        /**
         * can not join self
         */
        pthread_t tid = pthread_self();
        if (tid == _thread_id)
        {
            fprintf(stderr, "join slef, quit\n");
            //thread quit
            pthread_exit(NULL);
            return 0;
        }

        pthread_join(_thread_id, NULL);
        _thread_id = INVALID_THREAD_ID;
    }

    return 0;
}

void *PthreadThread::thread_fun(void *arg)
{
    ThreadInternal* tharg = (ThreadInternal*)arg;
    boost::shared_ptr<PthreadThread> *thread = reinterpret_cast<boost::shared_ptr<PthreadThread> *>(tharg->thread_arg);
    boost::shared_ptr<PthreadThread> this_thread = *thread;
    delete thread;

    int ret = ::write(tharg->fd[1], "", 1);
    (void)ret;

    if (this_thread == NULL)
        return (void *)0;

    if (this_thread->_state != STARTING)
        return (void *)0;

#ifdef CPU_AFFINITY
    if (this_thread->m_cpu_set.empty() == false)
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        for (size_t i = 0; i < this_thread->m_cpu_set.size(); ++i)
            CPU_SET(this_thread->m_cpu_set[i], &mask);

        //int ret = sched_setaffinity(0, sizeof(mask), &mask);
        ret = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
        if (ret != 0)
            fprintf(stderr, "set cpu affinity error: %s\n", strerror(errno));

#ifdef THREAD_DEBUG
        CPU_ZERO(&mask);
        ret = pthread_getaffinity_np(pthread_self(), sizeof(mask), &mask);
        if (ret != 0)
        {
            fprintf(stderr, "get cpu affinity error: %s\n", strerror(errno));
        }
        else
        {
            fprintf(stderr, "thread %lu will run on cpu [ ", pthread_self());
            for (int i = 0; i < CPU_SETSIZE; ++i)
                if (CPU_ISSET(i, &mask))
                    fprintf(stderr, "%d ", i);
            fprintf(stderr, "]\n");
        }
#endif
    }
#endif
    this_thread->_state = STARTED;
    this_thread->_state = RUNNING;
    this_thread->runnable()->run();
    this_thread->_state = STOPPING;
    return (void *)0;
}

}

boost::shared_ptr<Thread> PosixThreadFactory::newThread(boost::shared_ptr<Runnable> runnable) const
{
    assert("Runnable must not be NULL {app}" && runnable != NULL);

    boost::shared_ptr<PthreadThread> thread(
            new PthreadThread(toPthreadPolicy(_thread_policy),
                toPthreadPriority(_thread_priority, _thread_policy),
                _thread_stack_size,
                _thread_detached,
                runnable));
    thread->setBind(true);
    thread->weakRef(thread);
    runnable->thread(thread);

    return thread;
}

}
