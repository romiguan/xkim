#ifndef __CONN_POOL_FACTORY_H__
#define __CONN_POOL_FACTORY_H__
#include "singleton_adapter.h"
#include "conn_pool.h"
#include "pooled_client.h"
#include "lock.h"
#include "defines.h"
#include "zookeeper_client.h"
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>

namespace Pilot {
template<typename ClientType,typename ProxyType>
class ConnPoolFactory
{
public:
	ConnPoolFactory() {}
	~ConnPoolFactory();

public:
	/**
	* Function that gets a pointer of one PooledClientImpl instance.
	* If the path has already in the map,return directly;
	* Otherwise,create a new one,stores in the map and return.
	*
	* @param zk_clientp Pointer of one zookeeper client instance.
	* @param path String of the node's path that the service
	*	regists on the zookeeper server.
	* @param replica_id Id of the service,reserved for future.
	* @param num_clients_per_replica Count of the clients of 
	*	the service in the pool.
	* @param conn_timeout Connection timeout of the thrift client.
	* @param send_timeout Send timeout of the thrift client.
	* @param recv_timeout Recv timeout of the thrift client.
	* @return Pointer of one PooledClientImpl instance.
	*/
	PooledClientImpl<ClientType,ProxyType> *get_pooled_client(ZookeeperClient *zk_clientp,
											const string path,
											int32_t replica_id,
											int32_t num_clients_per_replica,
											int32_t conn_timeout,
											int32_t send_timeout,
											int32_t recv_timeout);


    PooledClientImpl<ClientType, ProxyType>* get_pooled_client_simple(ZookeeperClient* zk_clientp,
            const string& path,
            int32_t replica_id,
            int32_t num_clients_per_replica,
            int32_t conn_timeout,
            int32_t send_timeout,
            int32_t recv_timeout);

private:
	map<string,shared_ptr<PooledClientImpl<ClientType,ProxyType> > > conn_pool_factory_map_;
	Lock lock_;

//DECLARE_SINGLETON_CLASS(ConnPoolFactory);
};

};
#include "conn_pool_factory.inl"

#endif
