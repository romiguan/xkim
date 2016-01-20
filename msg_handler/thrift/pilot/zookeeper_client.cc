#include "zookeeper_client.h"
#include "watcher_action.h"

#include <string.h>

namespace Pilot {

ZookeeperClient::ZookeeperClient():
    host_(""),
    recv_timeout_(10000),
    zh_(NULL)
{
}

ZookeeperClient::~ZookeeperClient()
{
	close();
}

void ZookeeperClient::set_host(const std::string host)
{
	host_=host;
}

void ZookeeperClient::set_recv_timeout(int32_t recv_timeout)
{
	recv_timeout_=recv_timeout * 3;
}

void ZookeeperClient::set_watcher(WatcherAction *watcherp)
{
	watcherp_.reset(watcherp);
    assert(watcherp_ != NULL);
    //fprintf(stderr, "===========================%d\n", watcherp_.use_count());
}

const string ZookeeperClient::get_host()
{
	return host_;
}

const WatcherAction *ZookeeperClient::get_watcher()
{
	return watcherp_.get();
}

const zhandle_t *ZookeeperClient::get_raw_zhp()
{
	return zh_;
}

int32_t ZookeeperClient::open()
{
	MutexLock mlock(&lock_);
	if(zh_){
		fprintf(stderr,"zookeeper_init failed,zh_ already exit\n");
		return -2;
	}
	zh_=zookeeper_init(host_.c_str(),
			active_watcher,
			recv_timeout_,
			0,watcherp_.get(),0);

    assert(zh_ != NULL);
	zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

	return 0;
}

int32_t ZookeeperClient::close()
{
	MutexLock mlock(&lock_);
	int32_t rc=zookeeper_close(zh_);
	if(rc==ZOK){
		zh_=NULL;
	}
	return rc;
}

int32_t ZookeeperClient::reopen()
{
    MutexLock mlock(&lock_);
    zookeeper_close(zh_);
    zh_=zookeeper_init(host_.c_str(),
            active_watcher,
            recv_timeout_,
            0,watcherp_.get(),0);

    assert(zh_ != NULL);
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

    return 0;
}

int32_t ZookeeperClient::get_children(const char *path,std::vector<std::string> &children)
{
	struct String_vector 	str_vec;
	struct Stat				stat;
	int32_t 				rc;

	{
		MutexLock mlock(&lock_);
		if(!zh_){
			return -1;
		}
		rc=zoo_wget_children2(zh_,path,active_watcher,watcherp_.get(),&str_vec,&stat);
	}
	if(rc==ZOK){
		for(int32_t i=0;i<str_vec.count;++i){
			children.push_back(std::string(path)+"/"+std::string(str_vec.data[i]));
		}
	}

	return rc;
}

int32_t ZookeeperClient::get_nodevalue(const char *path,std::string &value)
{
	struct Stat	stat;
	int32_t		rc;
	char	 	val[MAX_VALUE_LEN]={'\0'};
	int32_t		val_len=MAX_VALUE_LEN;

	{
		MutexLock mlock(&lock_);
		if(!zh_){
			return -1;
		}
		rc=zoo_wget(zh_,path,active_watcher,watcherp_.get(),val,&val_len,&stat);
	}
	if(rc==ZOK){
		value=std::string(val);
	}

	return rc;
}

int32_t ZookeeperClient::node_exists(const char *path,bool &exists)
{
	struct Stat	stat;
	int32_t		rc;
	
	{
		MutexLock mlock(&lock_);
		if(!zh_){
			return -1;
		}
		rc=zoo_wexists(zh_,path,active_watcher,watcherp_.get(),&stat);
	}
	if(rc==ZOK){
		exists=true;
	}else{
		exists=false;
	}

	return rc;
}

int32_t ZookeeperClient::node_create(const char *path,const char *value,const struct ACL_vector *acl,int flags)
{
	int32_t		rc;
	char		val[MAX_VALUE_LEN]={'\0'};
	int32_t		val_len=MAX_VALUE_LEN;
	char		pth[MAX_PATH_LEN]={'\0'};
	int32_t		pth_len=MAX_PATH_LEN-1;

	if(strlen(value)>=MAX_VALUE_LEN){
		fprintf(stderr,"Value_length too big\n");
		return ZBADARGUMENTS;
	}
	strncpy(val,value,strlen(value));
	val_len=strlen(value);

	{
		MutexLock mlock(&lock_);
		if(!zh_){
			return -1;
		}
		rc=zoo_create(zh_,path,val,val_len,acl,flags,pth,pth_len);
	}

	return rc;
}

int32_t ZookeeperClient::node_delete(const char *path)
{
	int32_t		rc;

	{
		MutexLock mlock(&lock_);
		if(!zh_){
			return -1;
		}
		rc=zoo_delete(zh_,path,-1);
	}

	return rc;
}

}
