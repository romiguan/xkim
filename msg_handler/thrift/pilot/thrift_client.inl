namespace Pilot {
template<typename ClientType>
ThriftClient<ClientType>::ThriftClient(const string host,int32_t port,
	string tprotocol,
	int32_t conn_timeout,int32_t send_timeout,int32_t recv_timeout):
	host_(host),
	port_(port),
	tprotocol_(tprotocol),
	conn_timeout_(conn_timeout),
	send_timeout_(send_timeout),
	recv_timeout_(recv_timeout)
{
	socket_.reset(new TSocket(host_,port_));
	//transport_.reset(new TBufferedTransport(socket_));
	transport_.reset(new TFramedTransport(socket_));
	if(tprotocol_==TBINARY_PROTOCOL){
		protocol_.reset(new TBinaryProtocol(transport_));
	}else if(tprotocol_==TCOMPACT_PROTOCOL){
		protocol_.reset(new TCompactProtocol(transport_));
	}else if(tprotocol_==TJSON_PROTOCOL){
		protocol_.reset(new TJSONProtocol(transport_));
	}else if(tprotocol_==TDENSE_PROTOCOL){
		protocol_.reset(new TDenseProtocol(transport_));
	}else if(tprotocol_==TDEBUG_PROTOCOL){
		protocol_.reset(new TDebugProtocol(transport_));
	}else{
		tprotocol_=TBINARY_PROTOCOL;
		protocol_.reset(new TBinaryProtocol(transport_));
	}
	client_.reset(new ClientType(protocol_));

	socket_->setConnTimeout(conn_timeout_);
	socket_->setSendTimeout(send_timeout_);
	socket_->setRecvTimeout(recv_timeout_);

	replica_=make_pair(host,port);
}

template<typename ClientType>
ThriftClient<ClientType>::~ThriftClient()
{
	close_service();
}

template<typename ClientType>
shared_ptr<TTransport> ThriftClient<ClientType>::get_transport()
{
	return transport_;
}

template<typename ClientType>
shared_ptr<ClientType> ThriftClient<ClientType>::get_client()
{
	return client_;
}

template<typename ClientType>
pair<string,int32_t> ThriftClient<ClientType>::get_replica()
{
	return replica_;
}

template<typename ClientType>
int32_t ThriftClient<ClientType>::open_service()
{
	if(transport_->isOpen()){
		return 0;
	}

	int32_t ret=-1;
	try{
		transport_->open();
		ret=0;
	}catch(TException &tx){
		fprintf(stderr,"open_service() ERROR:%s\n",tx.what());
	}

	return ret;
}

template<typename ClientType>
int32_t ThriftClient<ClientType>::close_service()
{
	if(!transport_->isOpen()){
		return 0;
	}

	int32_t ret=-1;
	try{
		transport_->close();
		ret=0;
	}catch(TException &tx){
		fprintf(stderr,"close_service() ERROR:%s\n",tx.what());
	}

	return ret;
}
}
