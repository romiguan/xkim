namespace Pilot {
template<typename ClientType,typename ProxyType>
ConnPoolFactory<ClientType,ProxyType>::~ConnPoolFactory()
{
	MutexLock mlock(&lock_);
	conn_pool_factory_map_.erase(conn_pool_factory_map_.begin(),
								conn_pool_factory_map_.end());
}

template<typename ClientType,typename ProxyType>
PooledClientImpl<ClientType, ProxyType>* ConnPoolFactory<ClientType,ProxyType>::get_pooled_client_simple(
        ZookeeperClient* zk_clientp,
        const string& path,
        int32_t replica_id,
        int32_t num_clients_per_replica,
        int32_t conn_timeout,
        int32_t send_timeout,
        int32_t recv_timeout)
{
    (void)replica_id;
    PooledClientImpl<ClientType, ProxyType>* service = new PooledClientImpl<ClientType, ProxyType>(zk_clientp,
            path,
            num_clients_per_replica,
            conn_timeout,
            send_timeout,
            recv_timeout);

    assert(service != NULL);
    service->open();
    return service;
}

template<typename ClientType,typename ProxyType>
PooledClientImpl<ClientType,ProxyType> *ConnPoolFactory<ClientType,ProxyType>::get_pooled_client(
						ZookeeperClient *zk_clientp,
						const string path,
						int32_t replica_id,
						int32_t num_clients_per_replica,
						int32_t conn_timeout,
						int32_t send_timeout,
						int32_t recv_timeout)
{
    (void)replica_id;
	MutexLock mlock(&lock_);
    if(conn_pool_factory_map_.find(path)==conn_pool_factory_map_.end()){
        shared_ptr<PooledClientImpl<ClientType,ProxyType> > conn_pool(
                new PooledClientImpl<ClientType,ProxyType>(zk_clientp,path,
                    num_clients_per_replica,
                    conn_timeout,
                    send_timeout,
                    recv_timeout)
                );
        if (conn_pool->open() == 0)
            conn_pool_factory_map_.insert(make_pair(path,conn_pool));
        else
            return NULL;
    }
	return conn_pool_factory_map_[path].get();
}
}
