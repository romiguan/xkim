#ifndef __REGIST_SERVICE_H__
#define __REGIST_SERVICE_H__

#include "zookeeper_client.h"
#include "update_task_manager.h"
#include "defines.h"
#include "string_algo.h"
#include "exception.h"
#include "thrift_server.h"

#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>

namespace Pilot {

class ServerCluster
{
public:
	/**
	* Constructor
	*
	* It's not preferred to use this.Try use another one instead.
	*
	* @param zclientp Pointer of a zookeeper client
	* @param service_path String of the service path registed on
	*	zookeeper server.
	*/
	ServerCluster(const string service_path, const string service_protocol, int32_t service_port)
	{
		assert(service_path.empty() == false);
		assert(service_protocol.empty() == false);
		assert(service_port > 0 && service_port < 65535);
		
		host_ip_ = fetch_host();
        assert(host_ip_.empty() == false);

		char json[2048];
		size_t total_len=2048;
		size_t cur_pos=0;
		bzero(json,total_len);
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"{\"ip\":");
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"%s\",",host_ip_.c_str());
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"port\":");
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"%d,",service_port);
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"protocol\":");
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"%s\",",service_protocol.c_str());
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"status\":1}");
		service_value_ = json;

		cur_pos=0;
		bzero(json,total_len);
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"{\"ip\":");
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"%s\",",host_ip_.c_str());
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"port\":");
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"%d,",service_port);
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"protocol\":");
		cur_pos+=snprintf(json+cur_pos,total_len-cur_pos,"\"%s\"}",service_protocol.c_str());
		string service_node(json);

		string real_service_path(DEFAULT_NAMESPACE);
		if(real_service_path.c_str()[0]=='/'){
			real_service_path+=service_path;
		}else{
			real_service_path+="/";
			real_service_path+=service_path;
		}
		if(real_service_path.c_str()[real_service_path.size()-1]!='/'){
			real_service_path+="/";
		}
		real_service_path+="rpc/";
		real_service_path+=service_node;
		
		service_path_ = real_service_path;
		StringAlgo::tokenize(service_path_,"/",zk_nodes_);
		assert(zk_nodes_.size() > 0);
	};

	template<typename ServerType>
	int32_t join(ThriftServer<ServerType> *serverp){
		return 0;
	};

	/**
	* Function join a service into the cluster.
	* @param service_port Port of the service.
	* @param service_protocol Protocol of the service.
	*/
	int32_t join()
	{
		assert(zclientp_ != NULL);
		int32_t rc;
		bool exists;
		vector<string> &strs = zk_nodes_;
		
		string curr_path="";
		for(size_t i=0;i<strs.size();++i){
			curr_path+="/";
			curr_path+=strs[i];

            //fprintf(stderr, "join ++++++++++++++++++++:%s\n", curr_path.c_str());
			rc=zclientp_->node_exists(curr_path.c_str(),exists);
			if(rc==ZOK&&exists==true){
				if(i==strs.size()-1){
					string node_value;
					rc = zclientp_->get_nodevalue(curr_path.c_str(), node_value);
					if (rc == ZOK && service_value_ != node_value)
						rc = zclientp_->node_create(curr_path.c_str(),service_value_.c_str(),&ZOO_OPEN_ACL_UNSAFE,ZOO_EPHEMERAL);
					return rc != ZOK ? -1 : 0;
				}else{
					continue;
				}
			}
			
			if(i==strs.size()-1){
				rc=zclientp_->node_create(curr_path.c_str(),service_value_.c_str(),&ZOO_OPEN_ACL_UNSAFE,ZOO_EPHEMERAL);
			}else{
				rc=zclientp_->node_create(curr_path.c_str(),"jd_service",&ZOO_OPEN_ACL_UNSAFE,0);
			}

			if(rc!=ZOK){
				return -1;
			}
		}
		return 0;
	};

	~ServerCluster()
	{
		quit();
	};

	void set_zookeeper_client(ZookeeperClient *zclientp)
	{
		zclientp_ = zclientp;
	};
	
	const string get_server_node() const
	{
		return service_path_;
	}

    const string get_server_ip() const
    {
        return host_ip_;
    }

	void quit()
	{
		fprintf(stderr,"[DEBUG],quit\n");
		int32_t rc=zclientp_->node_delete(service_path_.c_str());
		fprintf(stderr,"[DEBUG],rc=%d\n",rc);
	};

private:
	ZookeeperClient *zclientp_;
	string service_path_;
	string service_value_;
	vector<string> zk_nodes_;
    string host_ip_;

private:
    string fetch_host()
    {
        char hostname[256] ;
        char **pptr;
        char str[32];
        string host;

        gethostname(hostname,sizeof(hostname));
        struct hostent *hptr=gethostbyname(hostname);    
        if (hptr) {
            switch(hptr->h_addrtype) {
                case AF_INET: 
                    pptr=hptr->h_addr_list;
                    for(;*pptr!=NULL;pptr++) {
                        memset(str, 0, sizeof(str));
                        inet_ntop(hptr->h_addrtype,*pptr,str,sizeof(str));
                        if (strncmp(str, "127.", 4) != 0) {
                            host = str;
                            break;
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        if (host.empty()) {
            struct ifreq iface;
            strcpy(iface.ifr_ifrn.ifrn_name, "eth0");
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            assert(fd != -1);
            if (ioctl(fd, SIOCGIFADDR, &iface) == 0)
            {
                close(fd);
                char ip[32];
                inet_ntop(AF_INET, &((struct sockaddr_in *)(&iface.ifr_ifru.ifru_addr))->sin_addr, ip, sizeof(ip));
                host = ip;
            }
        }

        if (host.empty()) {
            struct ifaddrs* ifAddrStruct=NULL;
            getifaddrs(&ifAddrStruct);

            while (ifAddrStruct != NULL)
            {
                if (ifAddrStruct->ifa_addr->sa_family == AF_INET)
                {
                    void* addrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
                    char addressBuffer[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, addrPtr, addressBuffer, INET_ADDRSTRLEN);
                    if (strncmp(addressBuffer, "127.", 4) != 0)
                    {
                        //printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer); 
                        host.assign(addressBuffer);
                        break;
                    }
                }

                ifAddrStruct = ifAddrStruct->ifa_next;
            }
        }

        return host;

    };
};

};
#endif
