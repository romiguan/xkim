namespace Pilot {
template<typename ClientType>
ConnPool<ClientType>::ConnPool(const string host,int32_t port,int32_t conn_num,
		string tprotocol,
		int32_t conn_timeout,int32_t send_timeout,int32_t recv_timeout):
	host_(host),
	port_(port),
	conn_num_(conn_num),
	tprotocol_(tprotocol),
	conn_timeout_(conn_timeout),
	send_timeout_(send_timeout),
	recv_timeout_(recv_timeout),
	error_cnt_(0)
{
	init();
}

template<typename ClientType>
ConnPool<ClientType>::~ConnPool()
{
	MutexLock mlock(&lock_);
	clear_();
}

template<typename ClientType>
bool ConnPool<ClientType>::refresh_()
{
	clear_();
	return init_()>0;
}

template<typename ClientType>
int32_t ConnPool<ClientType>::init_()
{
	int32_t conn_num=0;
	for(int32_t i=0;i<conn_num_;++i){
		conn_num=init_one_conn();
	}

	return conn_num;
}

template<typename ClientType>
int32_t ConnPool<ClientType>::init_one_conn()
{
	ThriftClient<ClientType> *conn=new ThriftClient<ClientType>(
			host_,port_,tprotocol_,conn_timeout_,send_timeout_,recv_timeout_
			);
	bool succ=false;
	if(conn->open_service()==0){
		release_client_(conn);
		succ=true;
	}
#if 0

	int32_t retry_time=3;
	struct timeval tv={1,0};
	bool succ=false;
	while(retry_time-->0){
		if(conn->open_service()==0){
			release_client(conn);
			succ=true;
			break;
		}else{
			if(retry_time>0){
				select(0,NULL,NULL,NULL,&tv);
			}
		}
	}
#endif

	if(!succ){
		delete conn;
		fprintf(stderr,"ConnPool::init_one_conn() failed\n");
	}

	return conn_list_.len();
}

template<typename ClientType>
ThriftClient<ClientType> *ConnPool<ClientType>::acquire_client_()
{
	ThriftClient<ClientType> *thrift_client=NULL;

	conn_pool_node *nodep=NULL;
	nodep=conn_list_.try_get();
	if(nodep){
		thrift_client=(ThriftClient<ClientType>*)(nodep->data);
		delete nodep;
	}

	return thrift_client;
}

template<typename ClientType>
void ConnPool<ClientType>::release_client_(ThriftClient<ClientType> *thrift_client)
{
	conn_pool_node *nodep=new conn_pool_node;
	nodep->data=(void*)thrift_client;
	if(conn_list_.len()>conn_num_){
		delete thrift_client;
		delete nodep;
	}else{
		conn_list_.put(*nodep);
	}
}

template<typename ClientType>
void ConnPool<ClientType>::clear_()
{
	ThriftClient<ClientType> *thrift_client;
	conn_pool_node *nodep;
	
	while((nodep=conn_list_.try_get())){
		thrift_client=(ThriftClient<ClientType>*)(nodep->data);
		delete nodep;

		if(thrift_client){
			delete thrift_client;
		}
	}
}

template<typename ClientType>
typename ConnPool<ClientType>::HealthyStatus ConnPool<ClientType>::is_healthy_()
{
	int32_t error_cnt;
	struct timeval last_ts;

	error_cnt=error_cnt_;
	last_ts=last_err_timestamp_;

	if(error_cnt==0){
		return HEALTHY;
	}else if(error_cnt>MAX_ERROR_CNT_THRESHOLD){
		return DEAD;
	}else{
		struct timeval now,ts,tscmp;
		gettimeofday(&now,NULL);
		ts.tv_usec=0;
		ts.tv_sec=error_cnt*error_cnt;
		timeval_add(&last_ts,&ts,&tscmp);
		if((tscmp.tv_sec>now.tv_sec) || (tscmp.tv_sec==now.tv_sec && tscmp.tv_usec>now.tv_usec)){
			return NOT_HEALTHY_AND_CANNOT_BE_USED_TEMP;
		}else{
			return NOT_HEALTHY_BUT_CAN_HAVEATRY;
		}
	}
}

template<typename ClientType>
int32_t ConnPool<ClientType>::init()
{
	MutexLock mlock(&lock_);
	return init_();
}

template<typename ClientType>
bool ConnPool<ClientType>::refresh()
{
	MutexLock mlock(&lock_);
	return refresh_();
}

template<typename ClientType>
ThriftClient<ClientType> *ConnPool<ClientType>::acquire_client()
{
	MutexLock mlock(&lock_);
	return acquire_client_();
}

template<typename ClientType>
void ConnPool<ClientType>::release_client(ThriftClient<ClientType> *thrift_client)
{
	MutexLock mlock(&lock_);
	release_client_(thrift_client);
}

template<typename ClientType>
void ConnPool<ClientType>::clear()
{
	MutexLock mlock(&lock_);
	clear_();
}

template<typename ClientType>
int32_t ConnPool<ClientType>::pool_size()
{
	MutexLock mlock(&lock_);
	return conn_list_.len();
}

template<typename ClientType>
typename ConnPool<ClientType>::HealthyStatus ConnPool<ClientType>::is_healthy()
{
	MutexLock mlock(&lock_);
	return is_healthy_();
}

template<typename ClientType>
typename ConnPool<ClientType>::HealthyStatus ConnPool<ClientType>::report_error()
{
	MutexLock mlock(&lock_);
	++error_cnt_;
	gettimeofday(&last_err_timestamp_,NULL);
	return is_healthy_();
}

template<typename ClientType>
void ConnPool<ClientType>::clear_error()
{
	MutexLock mlock(&lock_);
	if(is_healthy_()==HEALTHY){
		return;
	}
	error_cnt_=0;
}

}
