#ifndef __ZOOKEEPER_CLIENT_H__
#define __ZOOKEEPER_CLIENT_H__

extern "C"{
#include <zookeeper/zookeeper.h>
}
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <map>
#include "lock.h"

#define MAX_VALUE_LEN 	8192
#define MAX_PATH_LEN	8192

using std::string;

namespace Pilot {

class WatcherAction;
class ZookeeperClient
{
public:
	ZookeeperClient();
	~ZookeeperClient();

public:
	void 	set_host(const std::string host);
	void 	set_recv_timeout(int32_t recv_timeout);
	void 	set_watcher(WatcherAction *watcherp);
	const string 		get_host();
	const WatcherAction *get_watcher();
	const zhandle_t 	*get_raw_zhp();
    int32_t	open();
    int32_t reopen();
	int32_t close();
	int32_t get_children(const char *path,std::vector<std::string> &children);
	int32_t get_nodevalue(const char *path,std::string &value);
	int32_t node_exists(const char *path,bool &exists);
	int32_t node_create(const char *path,const char *value,const struct ACL_vector *acl,int flags);
	int32_t node_delete(const char *path);

private:
    boost::shared_ptr<WatcherAction> watcherp_;
	std::string		host_;
	int32_t			recv_timeout_;
	zhandle_t 		*zh_;
	Lock			lock_;
};

}
#endif
