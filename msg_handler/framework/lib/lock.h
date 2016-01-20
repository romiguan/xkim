#ifndef __LOCK_H__
#define __LOCK_H__

#include <pthread.h>

namespace util
{

class RWLock
{
    public:
        RWLock()
        {
            pthread_rwlock_init(&_rwlock, NULL);
        }

        ~RWLock()
        {
            pthread_rwlock_destroy(&_rwlock);
        }

        friend class ReaderLock;
        friend class WriterLock;

    private:
        pthread_rwlock_t _rwlock;
};

class Lock
{
    public:
        Lock()
        {
            pthread_mutex_init(&_lock, NULL);
        }

        ~Lock()
        {
            pthread_mutex_destroy(&_lock);
        }

        friend class MutexLock;

    private:
        pthread_mutex_t _lock;
};

class ReaderLock
{
    public:
        ReaderLock(RWLock* lock):
            _lock(lock)
        {
            pthread_rwlock_rdlock(&_lock->_rwlock);
        }

        ~ReaderLock()
        {
            pthread_rwlock_unlock(&_lock->_rwlock);
        }

    private:
        RWLock* _lock;
};

class WriterLock
{
    public:
        WriterLock(RWLock* lock):
            _lock(lock)
        {
            pthread_rwlock_wrlock(&_lock->_rwlock);
        }

        ~WriterLock()
        {
            pthread_rwlock_unlock(&_lock->_rwlock);
        }

    private:
        RWLock* _lock;
};

class MutexLock
{
    public:
        MutexLock(Lock* lock):
            _lock(lock)
        {
            pthread_mutex_lock(&_lock->_lock);
        }

        ~MutexLock()
        {
            pthread_mutex_unlock(&_lock->_lock);
        }

    private:
        Lock* _lock;
};

}

#endif
