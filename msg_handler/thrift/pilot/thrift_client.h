#ifndef __THRIFT_CLIENT_H__
#define __THRIFT_CLIENT_H__

#include <string>
#include <utility>
#include <boost/shared_ptr.hpp>

#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/protocol/TDenseProtocol.h>
#include <thrift/protocol/TDebugProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "defines.h"

using namespace std;
using namespace boost;

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
namespace Pilot {

template<typename ClientType>
class ThriftClient
{
public:
	typedef pair<string,int32_t> HostPort;
public:
	/**
	* Constructor
	*
	* @param host Host of the service.
	* @param port Port of the service.
	* @param tprotocol Protocol of the service.
	* @param conn_timeout Connection timeout.
	* @param send_timeout Send timeout.
	* @param recv_timeout Recv timeout.
	*/
	ThriftClient(const string host,int32_t port,string tprotocol,
		int32_t conn_timeout,int32_t send_timeout,int32_t recv_timeout);
	~ThriftClient();

public:
	/**
	* Method to get the transport.
	*
	* @return transport
	*/
	shared_ptr<TTransport> get_transport();
	/**
	* Method to get the client.
	*
	* return client
	*/
	shared_ptr<ClientType> get_client();
	/**
	* Method to open the connection.
	* 
	* @return 0 Succeed
		Otherwise Fail
	*/
	int32_t open_service();
	/**
	* Method to close the connection.
	*
	* @return 0 Succeed
	*	Otherwise Fail
	*/
	int32_t close_service();
	/**
	* Method to get the host and port pair.
	*
	* @return pair<host,port>
	*/
	HostPort get_replica();

private:
	std::string host_;
	int32_t 	port_;
	HostPort	replica_;
	string 		tprotocol_;
	int32_t		conn_timeout_;
	int32_t		send_timeout_;
	int32_t		recv_timeout_;
	shared_ptr<TSocket> 	socket_;
	shared_ptr<TTransport> 	transport_;
	shared_ptr<TProtocol> 	protocol_;
	shared_ptr<ClientType> 	client_;
};

};
#include "thrift_client.inl"
#endif
