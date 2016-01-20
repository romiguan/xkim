#include "string_algo.h"
#include "exception.h"
#include "singleton_adapter.h"
#include "conn_pool_factory.h"
#include "libjson/json.h"

#include <signal.h>
#include <arpa/inet.h>

namespace Pilot {

namespace
{

uint64_t genReplicId(const std::string& host, int port)
{
    unsigned int ip_binary;
    int ret = inet_pton(AF_INET, host.c_str(), &ip_binary);
    if (ret == 1)
    {
        uint64_t id = ip_binary;
        id = (id << 32) | port;
        return id;
    }
    else
    {
        return 0;
    }
}

}

template<typename ClientType,typename ProxyType>
PooledClientImpl<ClientType,ProxyType>::PooledClientImpl(ZookeeperClient *zk_clientp,
	const string path,
	int32_t num_clients_per_replica,
	int32_t conn_timeout,
	int32_t send_timeout,
	int32_t recv_timeout):
	num_clients_per_replica_(num_clients_per_replica),
	conn_timeout_(conn_timeout),
	send_timeout_(send_timeout),
	recv_timeout_(recv_timeout),
	path_(path),
	cur_replica_index_(0)
{
	thread_num_= 1;
	UpdateTaskManager::instance(path);
	zk_clientp_=zk_clientp;
	zk_server_=zk_clientp_->get_host();
}

template<typename ClientType,typename ProxyType>
PooledClientImpl<ClientType,ProxyType>::~PooledClientImpl()
{
	stop();//make sure the thread stopped,and all resource can be deleted without any issue...
	clear();
}

template<typename ClientType,typename ProxyType>
ThriftClient<ClientType> *PooledClientImpl<ClientType,ProxyType>::acquire_thrift_client()
{
	ThriftClient<ClientType> *result=NULL;

	{
		ReaderLock rlock(&rwlock_);
		for(size_t i=0;i<conn_pool_vec_.size();++i){
            int32_t host_idx = atomic_add(cur_replica_index_);
			ReplicaConnPool &rcp=conn_pool_vec_[host_idx % conn_pool_vec_.size()];
			if(rcp.status_==0){
				continue;
            }
			ConnPool<ClientType> *conn_pool_p=rcp.conn_pool_.get();
			typename ConnPool<ClientType>::HealthyStatus status=conn_pool_p->is_healthy();
			if(status!=ConnPool<ClientType>::HEALTHY &&
                status!=ConnPool<ClientType>::NOT_HEALTHY_BUT_CAN_HAVEATRY){
				continue;
            }
			if(status==ConnPool<ClientType>::NOT_HEALTHY_BUT_CAN_HAVEATRY){
				if(false==conn_pool_p->refresh()){
					conn_pool_p->report_error();
				}
			}
			ThriftClient<ClientType> *clientp=conn_pool_p->acquire_client();
			if(clientp && clientp->open_service()==0){
				result=clientp;
				break;
			}else if(clientp){
                clientp->open_service();
				conn_pool_p->release_client(clientp);
				continue;
			}
		}
	}
	//if(!result){
	//	throw TZException(TZE_NOVALID_CLIENT,0);
	//}

	return result;
}

template<typename ClientType,typename ProxyType>
ThriftClient<ClientType> *PooledClientImpl<ClientType,ProxyType>::acquire_thrift_client(uint64_t replica_id)
{
	ThriftClient<ClientType> *result=NULL;
	{
		ReaderLock rlock(&rwlock_);
        boost::unordered_map<uint64_t, int32_t>::iterator it = replica_map_.find(replica_id);
        if (it != replica_map_.end())
        {
			ReplicaConnPool &rcp=conn_pool_vec_[it->second];
            if(rcp.replica_id_ == replica_id && rcp.status_==1){
                ConnPool<ClientType> *conn_pool_p=rcp.conn_pool_.get();
                typename ConnPool<ClientType>::HealthyStatus status=conn_pool_p->is_healthy();
                if(status==ConnPool<ClientType>::NOT_HEALTHY_BUT_CAN_HAVEATRY){
                    if(false==conn_pool_p->refresh()){
                        conn_pool_p->report_error();
                    }
                    status=conn_pool_p->is_healthy();
                }

                if(status==ConnPool<ClientType>::HEALTHY)
                {
                    ThriftClient<ClientType> *clientp=conn_pool_p->acquire_client();
                    if(clientp && clientp->open_service()==0){
                        result=clientp;
                        return result;
                    }else if(clientp){
                        clientp->open_service();
                        conn_pool_p->release_client(clientp);
                        return NULL;
                    }
                }
            }
        }
    }
    return NULL;
}

template<typename ClientType,typename ProxyType>
bool PooledClientImpl<ClientType,ProxyType>::release_thrift_client(ThriftClient<ClientType> *thrift_client, ConnErrorStatus error)
{
	if(thrift_client==NULL){
		return true;
	}
	size_t i=0;
	bool flag=false;
	{
		ReaderLock rlock(&rwlock_);
		for(i=0;i<conn_pool_vec_.size();++i){
			if(conn_pool_vec_[i].backend_replica_==thrift_client->get_replica()){
				flag=true;
				break;
			}
		}
	}
	if(flag){
        if(error == CONN_OK){
            conn_pool_vec_[i].conn_pool_->clear_error();
            conn_pool_vec_[i].conn_pool_->release_client(thrift_client);
        } else if (error == CONN_DROP) {
            conn_pool_vec_[i].conn_pool_->report_error();
            delete thrift_client;
        } else if (error == CONN_RECONNECT) {
            thrift_client->close_service();
            thrift_client->open_service();
            //status=conn_pool_vec_[i].conn_pool_->report_error();
            conn_pool_vec_[i].conn_pool_->release_client(thrift_client);
        }

		return true;
	}
	if(!flag){
		delete thrift_client;
	}
	return false;
}

template<typename ClientType,typename ProxyType>
bool PooledClientImpl<ClientType,ProxyType>::release_thrift_client(uint64_t replica_id, ThriftClient<ClientType> *thrift_client, ConnErrorStatus error)
{
	if(thrift_client==NULL)
		return true;

	bool flag=false;
	{
		ReaderLock rlock(&rwlock_);
        boost::unordered_map<uint64_t, int32_t>::iterator it = replica_map_.find(replica_id);
        if (it != replica_map_.end())
        {
			ReplicaConnPool &rcp=conn_pool_vec_[it->second];
            if(rcp.replica_id_ == replica_id && rcp.backend_replica_==thrift_client->get_replica())
            {
                flag = true;
                if(error == CONN_OK){
                    rcp.conn_pool_->clear_error();
                    rcp.conn_pool_->release_client(thrift_client);
                } else {
                    thrift_client->close_service();
                    thrift_client->open_service();
                    rcp.conn_pool_->release_client(thrift_client);
                }

                return true;
            }
        }
	}

	if(!flag){
		delete thrift_client;
	}
	return false;
}

#if 0
template<typename ClientType,typename ProxyType>
void PooledClientImpl<ClientType,ProxyType>::report_error(ThriftClient<ClientType> *thrift_client)
{
	if(thrift_client==NULL){
		return;
	}
	typename ConnPool<ClientType>::HealthyStatus status=ConnPool<ClientType>::HEALTHY;
	typename vector<ReplicaConnPool>::iterator iter;
	{
		WriterLock rlock(&rwlock_);
		for(iter=conn_pool_vec_.begin();iter!=conn_pool_vec_.end();++iter){
			if(iter->backend_replica_==thrift_client->get_replica()){
				status=iter->conn_pool_->report_error();
				if(status==ConnPool<ClientType>::DEAD){
					conn_pool_vec_.erase(iter);
					return;
				}
			}
		}
	}
}

template<typename ClientType,typename ProxyType>
void PooledClientImpl<ClientType,ProxyType>::clear_error(ThriftClient<ClientType> *thrift_client)
{
	if(thrift_client==NULL){
		return;
	}
	{
		ReaderLock rlock(&rwlock_);
		for(size_t i=0;i<conn_pool_vec_.size();++i){
			if(conn_pool_vec_[i].backend_replica_==thrift_client->get_replica()){
				conn_pool_vec_[i].conn_pool_->clear_error();
				return;
			}
		}
	}
}
#endif

template <typename ClientType, typename ProxyType>
void PooledClientImpl<ClientType, ProxyType>::fixpath(string& path)
{
    if (path.empty())
        return;

    string tmp(DEFAULT_NAMESPACE);
    string user_path(path);

    if(user_path.c_str()[0]=='/'){
        tmp+=user_path;
    }else{
        tmp+="/";
        tmp+=user_path;
    }

    if(user_path.c_str()[user_path.size()-1]=='/'){
        tmp+="rpc";
    }else{
        tmp+="/rpc";
    }

    path = tmp;
}

template <typename ClientType, typename ProxyType>
PooledClientImpl<ClientType, ProxyType>* PooledClientImpl<ClientType, ProxyType>::instance_simple(ZookeeperClient* zk_clientp,
		const string& path,
		int32_t num_clients_per_replica,
		int32_t conn_timeout,
		int32_t send_timeout,
		int32_t recv_timeout)
{
	return SingletonAdapter<ConnPoolFactory<ClientType, ProxyType> >::instance()->get_pooled_client_simple(zk_clientp,
            path,
            -1,
            num_clients_per_replica,
            conn_timeout,
            send_timeout,
            recv_timeout);
}

template<typename ClientType,typename ProxyType>
PooledClientImpl<ClientType,ProxyType> *PooledClientImpl<ClientType,ProxyType>::instance(ZookeeperClient *zk_clientp,
		const string path,
		int32_t num_clients_per_replica,
		int32_t conn_timeout,
		int32_t send_timeout,
		int32_t recv_timeout)
{
    if (path.empty())
        return NULL;

	string tmp(DEFAULT_NAMESPACE);
	string user_path(path);

	if(user_path.c_str()[0]=='/'){
		tmp+=user_path;
	}else{
		tmp+="/";
		tmp+=user_path;
	}

	if(user_path.c_str()[user_path.size()-1]=='/'){
		tmp+="rpc";
	}else{
		tmp+="/rpc";
	}

	return SingletonAdapter<ConnPoolFactory<ClientType,ProxyType> >::instance()->get_pooled_client(zk_clientp,tmp,-1,num_clients_per_replica,conn_timeout,send_timeout,recv_timeout);
}

template<typename ClientType,typename ProxyType>
void PooledClientImpl<ClientType,ProxyType>::clear()
{
	WriterLock wlock(&rwlock_);
    conn_pool_vec_.clear();
    replica_map_.clear();
}

template<typename ClientType,typename ProxyType>
int32_t PooledClientImpl<ClientType,ProxyType>::open()
{
    /*
	UpdateTaskManager::UpdateTask utask;
	utask.task_type=UpdateTaskManager::REFRESH_ALL;
	utask.task_arg="";
	process_update_task(&utask);
    */
    UpdateTaskManager::instance(path_)->put_update_task(UpdateTaskManager::REFRESH_ALL, "");
	int32_t ret = open_thread();
	return ret;
}

template<typename ClientType,typename ProxyType>
int32_t PooledClientImpl<ClientType,ProxyType>::stop()
{
	for(int32_t i=0;i<thread_num_;++i){
		pthread_kill(thread_[i],SIGKILL);
        pthread_join(thread_[i], NULL);
	}
	free(thread_);
    thread_ = NULL;

	return 0;
}

template<typename ClientType,typename ProxyType>
int PooledClientImpl<ClientType,ProxyType>::svc()
{
	UpdateTaskManager::UpdateTask *taskp;

	while((taskp=UpdateTaskManager::instance(path_)->get_update_task())!=NULL){
		if(taskp->task_type==UpdateTaskManager::EXIT){
			break;
		}else if (taskp->task_type == UpdateTaskManager::TIMED_REFRESH_ALL){
            struct timeval tv1;
            gettimeofday(&tv1, NULL);
            uint32_t t = tv1.tv_sec * 1000 + tv1.tv_usec / 1000;
            if (t >= taskp->task_timed_) {
                taskp->task_type = UpdateTaskManager::REFRESH_ALL;
                process_update_task(taskp);
                delete taskp;
            } else {
                UpdateTaskManager::instance(path_)->put_update_task(taskp);
            }
		} else {
            process_update_task(taskp);
            delete taskp;
        }
	}

	return 0;
}

template<typename ClientType,typename ProxyType>
void PooledClientImpl<ClientType,ProxyType>::process_update_task(UpdateTaskManager::UpdateTask *taskp)
{
	switch(taskp->task_type){
		case UpdateTaskManager::REFRESH_ALL:
		{
            fprintf(stderr, "REFRESH_ALL: %s\n", path_.c_str());
			vector<ReplicaInfo> riv;
			int32_t replaca_count = get_all_replicas(riv);
			//zookeeper³ö´í,µÈ´ýzookeeper»Ö¸´
			if (replaca_count < 0)
				break;
			typename vector<ReplicaInfo>::iterator it;
			for(it=riv.begin();it!=riv.end();){
				if(it->status_==0){
					it=riv.erase(it);
				}else{
					++it;
                }
            }

            if(riv.size()==0){
                WriterLock wlock(&rwlock_);
				conn_pool_vec_.clear();
                replica_map_.clear();
                //UpdateTaskManager::instance(path_)->put_update_task(UpdateTaskManager::REFRESH_ALL, "");
                break;
            }

			{
                WriterLock wlock(&rwlock_);
                typename vector<ReplicaConnPool>::iterator iter;
                iter=conn_pool_vec_.begin();
                while(iter!=conn_pool_vec_.end()){
                    bool found=false;
                    for(size_t i=0;i<riv.size();++i){
                        if(iter->backend_replica_.first==riv[i].host_&&
                                iter->backend_replica_.second==riv[i].port_&&
                                iter->protocol_==riv[i].protocol_&&
                                iter->status_==riv[i].status_){
                            found=true;
                            break;
                        }
                    }
                    if(found){
                        ++iter;
                    }else{
                        replica_map_.erase(iter->replica_id_);
                        iter=conn_pool_vec_.erase(iter);
                    }
                }

				for(size_t i=0;i<riv.size();++i){
					bool found=false;
					for(size_t j=0;j<conn_pool_vec_.size();++j){
						if(conn_pool_vec_[j].backend_replica_.first==riv[i].host_&&
							conn_pool_vec_[j].backend_replica_.second==riv[i].port_&&
							conn_pool_vec_[j].protocol_==riv[i].protocol_&&
							conn_pool_vec_[j].status_==riv[i].status_){
							found=true;
							break;
						}
					}
					if(!found){
						ReplicaConnPool rcp;
						rcp.backend_replica_=make_pair(riv[i].host_,riv[i].port_);
						rcp.replica_id_=riv[i].replica_id_;
						rcp.protocol_=riv[i].protocol_;
						rcp.status_=riv[i].status_;
						rcp.conn_pool_.reset(
							new ConnPool<ClientType>(riv[i].host_,riv[i].port_,num_clients_per_replica_,
								riv[i].protocol_,conn_timeout_,send_timeout_,recv_timeout_)
						);
						if(rcp.conn_pool_->pool_size()>0){
                            rcp.index_ = (int)conn_pool_vec_.size();
                            replica_map_[rcp.replica_id_] = rcp.index_;
                            conn_pool_vec_.push_back(rcp);
						}
					}
				}
			}
			break;
		}
		case UpdateTaskManager::NODE_CREATED:
            fprintf(stderr, "NODE_CREATED: %s\n", path_.c_str());
		case UpdateTaskManager::CHILD_CHANGED:
		{
            fprintf(stderr, "CHILD_CHANGED: %s\n", path_.c_str());
			if(taskp->task_arg.size()==0||taskp->task_arg.compare(path_)!=0){
				return;
			}

			UpdateTaskManager::instance(path_)->put_update_task(UpdateTaskManager::REFRESH_ALL,"");
			break;
		}
		case UpdateTaskManager::NODE_VALUE_CHANGED:
		{
            fprintf(stderr, "NODE_VALUE_CHANGED: %s\n", path_.c_str());
			if(taskp->task_arg.size()==0||taskp->task_arg.compare(path_)==0){
				return;
			}

			ReplicaInfo ri;
			if(0==get_one_replica(taskp->task_arg.c_str(),ri)){
				return;
			}else{
				typename vector<ReplicaConnPool>::iterator iter;
				WriterLock wlock(&rwlock_);
				iter=conn_pool_vec_.begin();
				while(iter!=conn_pool_vec_.end()){
					if(iter->backend_replica_.first==ri.host_&&
						iter->backend_replica_.second==ri.port_&&
						iter->protocol_==ri.protocol_){
                        replica_map_.erase(iter->replica_id_);
						conn_pool_vec_.erase(iter);
						break;
					}else{
						++iter;
					}
				}

				if(ri.status_==1){
					ReplicaConnPool rcp;
					rcp.backend_replica_=make_pair(ri.host_,ri.port_);
					rcp.replica_id_=ri.replica_id_;
					rcp.protocol_=ri.protocol_;
					rcp.status_=ri.status_;
					rcp.conn_pool_.reset(
							new ConnPool<ClientType>(ri.host_,ri.port_,num_clients_per_replica_,
								ri.protocol_,conn_timeout_,send_timeout_,recv_timeout_)
							);
					if(rcp.conn_pool_->pool_size()>0){
                        rcp.index_ = (int)conn_pool_vec_.size();
                        replica_map_[rcp.replica_id_] = rcp.index_;
                        conn_pool_vec_.push_back(rcp);
					}
				}
			}

			break;
		}
		case UpdateTaskManager::NODE_DELETED:
		{
			fprintf(stderr,"NODE_DELETED according to the strategy,NODE_DELETED event will not launch any update\n");
			return;

			if(taskp->task_arg.size()==0){
				return;
			}else{
				if(taskp->task_arg.compare(path_)==0){
					WriterLock wlock(&rwlock_);
                    conn_pool_vec_.clear();
                    replica_map_.clear();
				}else{
					size_t pos=taskp->task_arg.rfind("/");
					if(pos!=std::string::npos){
						string p=taskp->task_arg.substr(0,pos);
						string v=taskp->task_arg.substr(pos+1);
						if(v.size()>0 && p.compare(path_)==0){
							ReplicaInfo ri;
							if(json_parse(v,ri)){
								typename vector<ReplicaConnPool>::iterator iter;
								WriterLock wlock(&rwlock_);
								iter=conn_pool_vec_.begin();
								while(iter!=conn_pool_vec_.end()){
									if(iter->backend_replica_.first==ri.host_&&
											iter->backend_replica_.second==ri.port_&&
											iter->protocol_==ri.protocol_){
                                        replica_map_.erase(iter->replica_id_);
                                        conn_pool_vec_.erase(iter);
										return;
									}else{
										++iter;
									}
								}
							}
						}
					}
					UpdateTaskManager::instance(path_)->put_update_task(UpdateTaskManager::REFRESH_ALL,"");
				}
			}
			break;
		}
		case UpdateTaskManager::EXIT:
		{
            fprintf(stderr, "EXIT: %s\n", path_.c_str());
			break;
		}
		default:
		{
			break;
		}
	}
}

template<typename ClientType,typename ProxyType>
int32_t PooledClientImpl<ClientType,ProxyType>::get_all_replicas(vector<ReplicaInfo> &riv)
{
	int32_t rc;
	bool exists;
	vector<string> children;
	string node_value;

	rc=zk_clientp_->node_exists(path_.c_str(),exists);
	if(!(rc==ZOK&&exists)){
		return -1;
	}

	rc=zk_clientp_->get_children(path_.c_str(),children);
	if(rc!=ZOK){
		return -1;
	}

	for(size_t i=0;i<children.size();++i){
		rc=zk_clientp_->get_nodevalue(children[i].c_str(),node_value);
		if(rc!=ZOK){
			return -1;
		}

		ReplicaInfo tri;
		if(json_parse(node_value,tri))
			riv.push_back(tri);
	}
	
	return (int32_t)riv.size();
}

template<typename ClientType,typename ProxyType>
int32_t PooledClientImpl<ClientType,ProxyType>::get_one_replica(const char *path,ReplicaInfo &ri)
{
	int32_t rc;
	bool exists;
	string node_value;

	rc=zk_clientp_->node_exists(path,exists);
    if(!(rc==ZOK&&exists)){
		return 0;
	}

	rc=zk_clientp_->get_nodevalue(path,node_value);
	if(rc!=ZOK||node_value.empty()){
		return 0;
	}
	
	ReplicaInfo tri;
	if(json_parse(node_value,tri)){
		ri=tri;
		return 1;
	}else{
		return 0;
	}
}

template<typename ClientType,typename ProxyType>
void *PooledClientImpl<ClientType,ProxyType>::monitor_proc(void *arg)
{
	PooledClientImpl<ClientType,ProxyType> *jdcpp=(PooledClientImpl<ClientType,ProxyType> *)arg;
	
	while(true){
		struct timeval tv={1800,0};
		select(0,NULL,NULL,NULL,&tv);
		UpdateTaskManager::instance(jdcpp->path_)->put_update_task(UpdateTaskManager::REFRESH_ALL,"");
	}

	return NULL;
}

template<typename ClientType,typename ProxyType>
void PooledClientImpl<ClientType,ProxyType>::start_monitor()
{
	pthread_t monitor_pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	if(pthread_create(&monitor_pid,&attr,monitor_proc,this)){
		fprintf(stderr,"start_monitor failed\n");
		throw TZException(TZE_FATAL,0);
	}
}

template<typename ClientType,typename ProxyType>
bool PooledClientImpl<ClientType,ProxyType>::json_parse(const string src,PooledClientImpl<ClientType,ProxyType>::ReplicaInfo &ri)
{
	Json::Reader 	reader;
	Json::Value		value;
	if(reader.parse(src,value)){
		ri.host_=value["ip"].asString();
		ri.port_=value["port"].asInt();
        ri.replica_id_ = genReplicId(ri.host_, ri.port_);
        if (ri.replica_id_ == 0)
        {
            fprintf(stderr, "%s: %d error\n", ri.host_.c_str(), ri.port_);
            return false;
        }
		ri.protocol_=value["protocol"].asString();
		ri.status_=value["status"].asInt();
		return true;
	}else{
		return false;
	}
}

template<typename ClientType,typename ProxyType>
shared_ptr<ProxyType> PooledClientImpl<ClientType,ProxyType>::acquire()
{
	return shared_ptr<ProxyType>(new (std::nothrow) ProxyType(this));
}

template<typename ClientType,typename ProxyType>
shared_ptr<ProxyType> PooledClientImpl<ClientType,ProxyType>::acquire(uint64_t replica_id)
{
	return shared_ptr<ProxyType>(new (std::nothrow) ProxyType(this, replica_id));
}

/*
template<typename ClientType,typename ProxyType>
void PooledClientImpl<ClientType,ProxyType>::release(ProxyType *proxyp)
{
	delete proxyp;
}
*/

template<typename ClientType,typename ProxyType>
int32_t PooledClientImpl<ClientType,ProxyType>::open_thread()
{
	int32_t ret=-1;
	int32_t i;

	do{
        int32_t expect_thread_num = thread_num_;

		if(thread_num_==0 || (thread_=(pthread_t*)malloc(thread_num_*sizeof(pthread_t)))==NULL){
            thread_num_ = 0;
            thread_ = NULL;
			break;
		}

		for(i=0;i<thread_num_;++i){
			if(pthread_create(thread_+i,NULL,run_svc,this)){
                thread_num_ = i;
				break;
			}
		}

        if(thread_num_ == 0){
			break;
		}

        if (thread_num_ != expect_thread_num)
            fprintf(stderr,
                    "PooledClientImpl<ClientType,ProxyType>::open_thread thread count expect %d > current %d\n",
                    expect_thread_num, thread_num_);

		ret=0;
	}while(false);

	return ret;
}

template<typename ClientType,typename ProxyType>
void *PooledClientImpl<ClientType,ProxyType>::run_svc(void *arg)
{
	PooledClientImpl<ClientType,ProxyType> *taskp=(PooledClientImpl<ClientType,ProxyType> *)arg;
	taskp->svc();
	return NULL;
}

}

