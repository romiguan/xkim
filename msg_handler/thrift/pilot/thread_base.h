#ifndef __THREAD_BASE_H__
#define __THREAD_BASE_H__
#include <pthread.h>
#include <stdlib.h>

class ThreadBase 
{
public:
	ThreadBase();
	virtual ~ThreadBase();

	virtual int open()=0;
	virtual int activate();
	virtual int stop()=0;
	virtual int svc()=0;
	virtual int join();

private:
	static void* run(void *arg);

protected:
	pthread_t *thread_;
	size_t thread_num_;
	pthread_barrier_t barrier_;
};

#endif
