#ifndef __POOLED_CLIENT_H__
#define __POOLED_CLIENT_H__

#include "thrift_client.h"
#include "conn_pool.h"
#include "lock.h"
#include "defines.h"
#include "thread_base.h"
#include "wait_list.h"
#include "zookeeper_client.h"
#include "update_task_manager.h"

#include <pthread.h>

#include <vector>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

using namespace std;
using namespace boost;

#define PooledClient(Service) PooledClientImpl<Service##Client,Service##Proxy>
#define atomic_add(v) __sync_fetch_and_add(&v, 1)

namespace Pilot {

template<typename ClientType,typename ProxyType>
class ConnPoolFactory;
template<typename ClientType,typename ProxyType>
class ThriftProxy;

template<typename ClientType,typename ProxyType>
class PooledClientImpl:private boost::noncopyable
{
	typedef pair<string,int32_t> HostPort;
	struct ReplicaInfo
	{
		string host_;
		int32_t port_;
		uint64_t replica_id_;
		string protocol_;
		int32_t status_;
	};
	struct ReplicaConnPool
	{
		HostPort backend_replica_;
		uint64_t replica_id_;
        int32_t index_;
		string protocol_;
		int32_t status_;
		shared_ptr<ConnPool<ClientType> > conn_pool_;
	};
	typedef vector<ReplicaConnPool> ReplicaConnPoolVec;

friend class ConnPoolFactory<ClientType,ProxyType>;
public:
	~PooledClientImpl();

	//ProxyType *acquire();
	//void release(ProxyType *);
	shared_ptr<ProxyType> acquire();
	shared_ptr<ProxyType> acquire(uint64_t replica_id);

	/**
	* Function to get one pointer of a thrift client instance
	* from the connection pool.
	*
	* @return Pointer of a thrift client instance.
	*/
	ThriftClient<ClientType> *acquire_thrift_client();
	ThriftClient<ClientType> *acquire_thrift_client(uint64_t replica_id);
	/**
	* Function to redraw a thrift client intance,back into the
	* connection pool.
	*
	* @param thrift_client Pointer of the client that wants been
	* 	redrawn.
	* @return true Succeed;
	*	false Failed,usually because that the service's already
	*	been offline.
	*/
	bool release_thrift_client(ThriftClient<ClientType> *thrift_client, ConnErrorStatus error);
	bool release_thrift_client(uint64_t replica_id, ThriftClient<ClientType> *thrift_client, ConnErrorStatus error);
#if 0
	/**
	* Function to report error
	* @param thrift_client Pointer of the client that has error.
	*/
	void report_error(ThriftClient<ClientType> *thrift_client);
	/**
	* Function to clear error
	* @param thrift_client Pointer of the client that clears error.
	*/
	void clear_error(ThriftClient<ClientType> *thrift_client);
#endif
	/**
	* Static method for users to get an instance of PooledClientImpl.
	*
	* Users should not use this unless it's really necessary to 
	* create a specific zookeeper client by users self.
	*
	* @param zk_clientp Pointer of one zookeeper client instance.
	* @param path String of the node's path that the service regists 
	*	on the zookeeper server.
	* @param num_clients_per_replica num_clients_per_replica Count of 
	*	the clients of the service in the pool.
	* @param conn_timeout Connection timeout of the thrift client.
	* @param send_timeout Send timeout of the thrift client.
	* @param recv_timeout Recv timeout of the thrift client.
	* @return Pointer of one PooledClientImpl instance.
	*/
	static PooledClientImpl<ClientType,ProxyType> *instance(ZookeeperClient *zk_clientp,
		const string path,
		int32_t num_clients_per_replica=NUM_CLIENTS_PER_REPLICA,
		int32_t conn_timeout=CONN_TIMEOUT,
		int32_t send_timeout=SEND_TIMEOUT,
		int32_t recv_timeout=RECV_TIMEOUT);

    /**
     * just create one instance, not any more logic
     */
    static PooledClientImpl<ClientType, ProxyType>* instance_simple(ZookeeperClient* zk_clientp,
            const string& path,
            int32_t num_clients_per_replica=NUM_CLIENTS_PER_REPLICA,
            int32_t conn_timeout=CONN_TIMEOUT,
            int32_t send_timeout=SEND_TIMEOUT,
            int32_t recv_timeout=RECV_TIMEOUT);

    /**
     * fix path to satisfy zonde format
     * for instance_simple, you should this before
     */
    static void fixpath(string& path);

	void clear();

	/**
	* Thread proc that refresh the connection pool every 30 minutes.
	*/
	static void *monitor_proc(void *arg);
	static void *run_svc(void *);
	void start_monitor();

	std::string name_space(){
		return DEFAULT_NAMESPACE;
	};

	void set_num_clients_per_replica(int32_t v){
		num_clients_per_replica_=v;
	};

	void set_conn_timeout(int32_t v){
		conn_timeout_=v;
	};

	void set_send_timeout(int32_t v){
		send_timeout_=v;
	};

	void set_recv_timeout(int32_t v){
		recv_timeout_=v;
	};

	int32_t stop();
	int32_t open();
private:
	ReplicaConnPoolVec conn_pool_vec_;
    boost::unordered_map<uint64_t, int32_t> replica_map_;

	RWLock rwlock_;
	int32_t num_clients_per_replica_;
	int32_t conn_timeout_;
	int32_t send_timeout_;
	int32_t recv_timeout_;
	string zk_server_;
	string path_;
	ZookeeperClient *zk_clientp_;

	int32_t thread_num_;
	pthread_t *thread_;

	volatile int32_t cur_replica_index_;

private:
	int32_t open_thread();
	int32_t svc();

private:
	PooledClientImpl(ZookeeperClient *zk_clientp,const string path,int32_t num_clients_per_replica,
			int32_t conn_timeout,int32_t send_timeout,int32_t recv_timeout);

protected:
	void process_update_task(UpdateTaskManager::UpdateTask *task);
	int32_t get_all_replicas(vector<ReplicaInfo> &riv);
	int32_t get_one_replica(const char *path,ReplicaInfo &ri);
	bool json_parse(const string src,ReplicaInfo &ri);
};


};

#include "pooled_client.inl"

#endif
