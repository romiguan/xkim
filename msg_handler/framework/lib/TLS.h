#ifndef _TLS_H
#define _TLS_H

#include <pthread.h>

extern "C" {
    typedef void(* TLS_THR_DEST) (void *);
}

namespace util {
template <class TYPE>
class TLS
{
public:
    TLS (TYPE *ts_obj = 0);

    virtual ~TLS (void);

    TYPE *operator-> () const;

    /// Dump the state of an object.
    void dump (void) const;

    /// Actually implements the code that retrieves the object from
    /// thread-specific storage.
    TYPE *ts_get (void) const;

protected:

    /// Factors out common code for initializing TSS.  This must NOT be
    /// called with the lock held...
    int ts_init (void);

    /// Avoid race conditions during initialization.
    pthread_mutex_t keylock_;

    /// "First time in" flag.
    volatile int once_;

    /// Key for the thread-specific error data.
    pthread_key_t key_;

    /// "Destructor" that deletes internal TYPE * when thread exits.
    static void cleanup (void *ptr);
};

template <class TYPE>
TLS<TYPE>::TLS(TYPE *ts_obj)
    : once_ (0),
      key_ (0)
{
    // If caller has passed us a non-NULL TYPE *, then we'll just use
    // this to initialize the thread-specific value.  Thus, subsequent
    // calls to operator->() will return this value.  This is useful
    // since it enables us to assign objects to thread-specific data
    // that have arbitrarily complex constructors!

    int ret;

    pthread_mutex_init(&this->keylock_, 0);

    if (ts_obj != 0)
    {
        if ((ret = this->ts_init ()) != 0)
        {
            return;
        }

        if ((ret = pthread_setspecific(this->key_,
                                       (void *) ts_obj)) != 0)
        {
            return;
        }
    }
}

template <class TYPE>
TLS<TYPE>::~TLS (void)
{
    int ret;

    pthread_mutex_destroy(&this->keylock_);

    if (this->once_ != 0)
    {
        ret = pthread_key_delete (this->key_);
        if (ret != 0)
        {
            return;
        }
    }
}

template <class TYPE> TYPE *
TLS<TYPE>::operator-> () const
{
    return this->ts_get ();
}

template <class TYPE> void
TLS<TYPE>::dump (void) const
{
}

template <class TYPE> void
TLS<TYPE>::cleanup (void *ptr)
{
    // Cast this to the concrete TYPE * so the destructor gets called.
    delete (TYPE *) ptr;
}

template <class TYPE> int
TLS<TYPE>::ts_init (void)
{
    int ret;
    pthread_mutex_lock(&this->keylock_);

    // Use the Double-Check pattern to make sure we only create the key
    // once!
    if (this->once_ == 0)
    {
        if ((ret = pthread_key_create (&this->key_,
                                       (TLS_THR_DEST)&TLS<TYPE>::cleanup)) != 0)
        {
            pthread_mutex_unlock(&this->keylock_);
            return ret; // Major problems, this should *never* happen!
        }
        else
        {
            // This *must* come last to avoid race conditions!
            this->once_ = 1;
            pthread_mutex_unlock(&this->keylock_);
            return 0;
        }
    }

    pthread_mutex_unlock(&this->keylock_);
    return 0;
}

template <class TYPE> TYPE *
TLS<TYPE>::ts_get (void) const
{
    int ret;

    if (this->once_ == 0)
    {
        // Create and initialize thread-specific ts_obj.
        if ((ret = const_cast< TLS < TYPE > * >(this)->ts_init ()) != 0)
        {
            // Seriously wrong..
            return 0;
        }
    }

    TYPE *ts_obj = 0;

    // Get the ts_obj from thread-specific storage.  Note that no locks
    // are required here...
    void *temp = ts_obj; // Need this temp to keep G++ from complaining.
    temp = pthread_getspecific (this->key_);
    ts_obj = static_cast <TYPE *> (temp);

    // Check to see if this is the first time in for this thread.
    if (ts_obj == 0)
    {
        // Allocate memory off the heap and store it in a pointer in
        // thread-specific storage (on the stack...).

        ts_obj = new TYPE();

        if (ts_obj == 0)
            return 0;

        // Store the dynamically allocated pointer in thread-specific
        // storage.
        if ((ret = pthread_setspecific (this->key_,
                                        (void *) ts_obj)) != 0)
        {
            delete ts_obj;
            return 0; // Major problems, this should *never* happen!
        }
    }
    return ts_obj;
}
}
#endif
