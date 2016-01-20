#ifndef __THRIFT_SERVER_H__
#define __THRIFT_SERVER_H__
#include <pthread.h>
#include <signal.h>

namespace Pilot {

template<typename ServerType>
class ThriftServer
{
public:
	ThriftServer(ServerType *serverp){
		serverp_=serverp;
	};
	~ThriftServer(){
	};

public:
	void start(){
		pthread_create(&pid_,NULL,thread_proc,this);
		struct timeval tv={1,0};
		select(0,NULL,NULL,NULL,&tv);
	};

	void wait(){
		pthread_join(pid_,NULL);
	};

	void stop(){
		serverp_->stop();
		pthread_kill(pid_,SIGTERM);
	};

private:
	ServerType *serverp_;
	pthread_t pid_;

private:
	static void *thread_proc(void *arg){
		ThriftServer *thrift_serverp=(ThriftServer *)arg;
		thrift_serverp->serverp_->serve();
		return NULL;
	};

};

};

#endif
