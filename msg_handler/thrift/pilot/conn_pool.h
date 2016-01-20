#ifndef __CONN_POOL_H__
#define __CONN_POOL_H__

#include "unsafe_wait_list.h"
#include "thrift_client.h"
#include "lock.h"
#include <string>

using namespace std;
namespace Pilot{

enum ConnErrorStatus
{
    CONN_OK = 0,
    CONN_DROP,
    CONN_RECONNECT,
};

template<typename ClientType>
class ConnPool
{
public:
	enum HealthyStatus{
		HEALTHY=0,
		NOT_HEALTHY_BUT_CAN_HAVEATRY,
		NOT_HEALTHY_AND_CANNOT_BE_USED_TEMP,
		DEAD
	};


public:
	/**
	* Constructor
	*
	* @param host Host of the thrift server.
	* @param port Port of the thrift server.
	* @param num_conn Count of the connection in the pool.
	* @param tprotocol Protocol that the client will use,see @file defines.h
	* @param conn_timeout Connection timeout.
	* @param send_timeout Send timeout.
	* @param recv_timeout Recv timeout.
	*/
	ConnPool(const string host,int32_t port,int32_t num_conn,
		string tprotocol,
		int32_t conn_timeout,int32_t send_timeout,int32_t recv_timeout);
	~ConnPool();

public:
	int32_t init();
	/**
	* Function that gets one client's pointer.
	*
	* @return Pointer of one client.
	*/
	ThriftClient<ClientType> *acquire_client();
	/**
	* Function that redraws one client back into pool.
	*
	* @param clientp Pointer of client that wants to be redrawn.
	*/
	void release_client(ThriftClient<ClientType> *);
	/**
	* Function that gets current size of the pool.
	*/
	int32_t pool_size();
	/**
	* Function that value the healthy status.
	*/
	HealthyStatus is_healthy();
	/**
	* Function that reports error.
	*/
	HealthyStatus report_error();
	/**
	* Function that clear the error suspect.
	*/
	void clear_error();

	bool refresh();

	void clear();
protected:
	struct conn_pool_node
	{
		linked_list_node_t list_node;
		void *data;
	};

private:
	int32_t init_one_conn();
	HealthyStatus is_healthy_();
	void release_client_(ThriftClient<ClientType> *);
	ThriftClient<ClientType> *acquire_client_();
	int32_t init_();
	void clear_();
	bool refresh_();

private:
	string	host_;
	int32_t	port_;
	int32_t conn_num_;
	string tprotocol_;
	int32_t conn_timeout_;
	int32_t send_timeout_;
	int32_t recv_timeout_;
	unsafe_wait_list_t<conn_pool_node,&conn_pool_node::list_node> conn_list_;
	int32_t error_cnt_;
	struct timeval last_err_timestamp_;
	//RWLock 	rwlock_;
	Lock	lock_;
};

};
#include "conn_pool.inl"

#endif
