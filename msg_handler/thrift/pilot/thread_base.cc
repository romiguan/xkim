#include "thread_base.h"
#include <signal.h>

ThreadBase::ThreadBase():thread_(NULL),thread_num_(0)
{
}

ThreadBase::~ThreadBase()
{
	//join();
}

int ThreadBase::open()
{
	int ret=-1;
	size_t thread_num=1;
	size_t i;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	do{
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

		//if (pthread_attr_setstacksize(&attr, stack_size)){
		//	break;
		//}

		if(thread_num==0 || (thread_=(pthread_t*)malloc(thread_num*sizeof(pthread_t)))==NULL){
			break;
		}

		pthread_barrier_init(&barrier_,NULL,thread_num+1);
		for(i=0;i<thread_num;i++){
			if(pthread_create(thread_+i,&attr,run,this)){
				break;
			}
		}

		if((thread_num_=i)!=thread_num){
			break;
		}

		ret=0;
	}while(false);

	pthread_attr_destroy(&attr);
	return ret;
}

int ThreadBase::activate()
{
	pthread_barrier_wait(&barrier_);
	return 0;
}

int ThreadBase::join()
{
	if(thread_){
		for(size_t i=0;i<thread_num_;i++){
			pthread_kill(thread_[i],SIGTERM);
			pthread_join(thread_[i],NULL);
		}
		free(thread_);
		thread_=NULL;
		pthread_barrier_destroy(&barrier_);
	}
	return 0;
}

void* ThreadBase::run(void *arg)
{
	ThreadBase *task=(ThreadBase *)arg;
	pthread_barrier_wait(&task->barrier_);
	task->svc();
	return NULL;
}
