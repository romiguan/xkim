#ifndef __THRIFT_PROXY_H__
#define __THRIFT_PROXY_H__

#include "stdio.h"
#include "pooled_client.h"
#include "exception.h"
#include <sys/types.h>

namespace Pilot {

#define INVOKE_AND_RETURN_ONE_SHOT_BY_ID(cname,pname,...)\
do{\
    ThriftClient<cname##Client> *clientp=pooled_clientp_->acquire_thrift_client(id_);\
    ThriftProxyHelper<cname##Client,cname##Proxy> helper(clientp,pooled_clientp_,id_);\
    try {\
        if(clientp) {\
            return clientp->get_client()->pname(__VA_ARGS__);\
        } else {\
            throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NOSERVICE);\
        }\
    } catch (TTransportException &e) {\
        TTransportException::TTransportExceptionType etype = e.getType();\
        if (etype == TTransportException::TIMED_OUT) {\
            helper.set_error(CONN_RECONNECT);\
            throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_TIMEOUT);\
        } else if (etype == TTransportException::NOT_OPEN) {\
            /*just drop, zookeeper will notify reconnect when the service is ok*/\
            helper.set_error(CONN_DROP);\
            throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_PEERRESET);\
        } else {\
            helper.set_error(CONN_RECONNECT);\
            throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_OTHER);\
        }\
    } catch (TApplicationException &tae) {\
        /*the peer emit exception, left conn ok*/\
        throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_APP);\
    } catch (TException &te) {\
        helper.set_error(CONN_RECONNECT);\
        throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_OTHER);\
    }\
}while(false)

#define INVOKE_AND_RETURN_ONE_SHOT(cname,pname,...)\
do{\
    ThriftClient<cname##Client> *clientp=pooled_clientp_->acquire_thrift_client();\
    ThriftProxyHelper<cname##Client,cname##Proxy> helper(clientp,pooled_clientp_);\
    try {\
        if(clientp) {\
            return clientp->get_client()->pname(__VA_ARGS__);\
        } else {\
            throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NOSERVICE);\
        }\
    } catch (TTransportException &e) {\
        TTransportException::TTransportExceptionType etype = e.getType();\
        if (etype == TTransportException::TIMED_OUT) {\
            helper.set_error(CONN_RECONNECT);\
            throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_TIMEOUT);\
        } else if (etype == TTransportException::NOT_OPEN) {\
            /*just drop, zookeeper will notify reconnect when the service is ok*/\
            helper.set_error(CONN_DROP);\
            throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_PEERRESET);\
        } else {\
            helper.set_error(CONN_RECONNECT);\
            throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_OTHER);\
        }\
    } catch (TApplicationException &tae) {\
        /*the peer emit exception, left conn ok*/\
        throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_APP);\
    } catch (TException &te) {\
        helper.set_error(CONN_RECONNECT);\
        throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_OTHER);\
    }\
}while(false)

#define INVOKE_AND_RETURN_WITHIN_3_ATTEMPTS(cname,pname,...)\
do{\
    int32_t retry=3;\
    while(retry-->0){\
        ThriftClient<cname##Client> *clientp=pooled_clientp_->acquire_thrift_client();\
        ThriftProxyHelper<cname##Client,cname##Proxy> helper(clientp,pooled_clientp_);\
        /*fprintf(stderr,"[DEBUG %d] Try get one service for [%s]\n",3-retry, #pname);*/\
        try{\
            if(clientp){\
                return clientp->get_client()->pname(__VA_ARGS__);\
            }else if(retry==0){\
                throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NOSERVICE);\
            }\
        } catch (TTransportException &e) {\
            TTransportException::TTransportExceptionType etype = e.getType();\
            if (etype == TTransportException::TIMED_OUT) {\
                helper.set_error(CONN_RECONNECT);\
                if(retry==0){\
                    throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_TIMEOUT);\
                } else {\
                    continue;\
                }\
            } else if (etype == TTransportException::NOT_OPEN) {\
                /*just drop, zookeeper will notify reconnect when the service is ok*/\
                helper.set_error(CONN_DROP);\
                if(retry==0){\
                    throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_PEERRESET);\
                } else {\
                    continue;\
                }\
            } else {\
                helper.set_error(CONN_RECONNECT);\
                if(retry==0){\
                    throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_NET_OTHER);\
                } else {\
                    continue;\
                }\
            }\
        } catch (TApplicationException &tae) {\
            /*the peer emit exception, left conn ok*/\
            if(retry==0){\
                throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_APP);\
            }else{\
                continue;\
            }\
        } catch (TException &te){\
            helper.set_error(CONN_RECONNECT);\
            if(retry==0){\
                throw TZException(TZE_PROXY_INVOKE_FAILED, TZException::TZE_OTHER);\
            }else{\
                continue;\
            }\
        }\
    }\
}while(false)

#define DECLARE_THRIFT_PROXY(cname) private:\
    uint64_t id_; \
	PooledClientImpl<cname##Client,cname##Proxy> *pooled_clientp_;\
	public:\
	cname##Proxy(Pilot::PooledClientImpl<cname##Client,cname##Proxy> *ptr): \
        id_(0), \
        pooled_clientp_(ptr) \
    {} \
	cname##Proxy(Pilot::PooledClientImpl<cname##Client,cname##Proxy> *ptr, uint64_t id): \
        id_(id), \
        pooled_clientp_(ptr) \
    {}

template<typename ClientType,typename ProxyType>
class ThriftProxyHelper
{
public:
	typedef ClientType __ClientType;
	typedef ProxyType __ProxyType;

public:
	ThriftProxyHelper(ThriftClient<ClientType> *tcp,PooledClientImpl<ClientType,ProxyType> *pcp):
        thrift_clientp_(tcp),
        pooled_clientp_(pcp),
        id_(0),
        error_(CONN_OK)
    {
	}

	ThriftProxyHelper(ThriftClient<ClientType> *tcp,PooledClientImpl<ClientType,ProxyType> *pcp, uint64_t id):
        thrift_clientp_(tcp),
        pooled_clientp_(pcp),
        id_(id),
        error_(CONN_OK)
    {
	}

	~ThriftProxyHelper()
    {
		if(pooled_clientp_&&thrift_clientp_){
#if 0
			if(has_error_){
				pooled_clientp_->report_error(thrift_clientp_);
			}else{
				pooled_clientp_->clear_error(thrift_clientp_);
			}
#endif
			bool ret = (id_ == 0) ? pooled_clientp_->release_thrift_client(thrift_clientp_, error_) : pooled_clientp_->release_thrift_client(id_, thrift_clientp_, error_);
            (void)ret;
		}
	}

	void set_error(ConnErrorStatus error)
    {
        error_ = error;
	};

protected:
	ThriftClient<ClientType> *thrift_clientp_;
	PooledClientImpl<ClientType,ProxyType> *pooled_clientp_;
    uint64_t id_;
    ConnErrorStatus error_;
};

};

#endif
