#ifndef __LOCK_H__
#define __LOCK_H__
#include <pthread.h>

namespace Pilot {

class RWLock;
class Lock;
class ReaderLock;
class WriterLock;
class MutexLock;


class RWLock
{
public:
	RWLock(){
		pthread_rwlock_init(&rwlock_,NULL);
	};
	~RWLock(){
		pthread_rwlock_destroy(&rwlock_);
	};
friend class ReaderLock;
friend class WriterLock;

private:
	pthread_rwlock_t	rwlock_;
};

class Lock
{
public:
	Lock(){
		pthread_mutex_init(&lock_,NULL);
	};
	~Lock(){
		pthread_mutex_destroy(&lock_);
	};
friend class MutexLock;

private:
	pthread_mutex_t 	lock_;
};

class ReaderLock
{
public:
	ReaderLock(RWLock *lockp):lockp_(lockp){
		pthread_rwlock_rdlock(&lockp_->rwlock_);
	};
	~ReaderLock(){
		pthread_rwlock_unlock(&lockp_->rwlock_);
	};

private:
	RWLock *lockp_;
};

class WriterLock
{
public:
	WriterLock(RWLock *lockp):lockp_(lockp){
		pthread_rwlock_wrlock(&lockp_->rwlock_);
	};
	~WriterLock(){
		pthread_rwlock_unlock(&lockp_->rwlock_);
	};

private:
	RWLock *lockp_;
};

class MutexLock
{
public:
	MutexLock(Lock *lockp):lockp_(lockp){
		pthread_mutex_lock(&lockp_->lock_);
	};
	~MutexLock(){
		pthread_mutex_unlock(&lockp_->lock_);
	};

private:
	Lock *lockp_;
};

}

#endif
