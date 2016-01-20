#ifndef __WATCHER_ACTION_H__
#define __WATCHER_ACTION_H__
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
extern "C"{
#include <zookeeper/zookeeper.h>
}
#include "lock.h"
#include "defines.h"
#include "zookeeper_client.h"
#include "server_cluster.h"

using namespace std;
using namespace boost;

namespace Pilot {

/**
* @class WatcherAction Base class of the watcheraction
*/
class WatcherAction
{
protected:
    ZookeeperClient *owner_;

public:
	WatcherAction(){owner_ = NULL;}
	virtual ~WatcherAction(){};

public:
    void set_owner(ZookeeperClient *zk) { owner_ = zk; }

	virtual void on_session_expired(){
		owner_->reopen();
		fprintf(stderr,"on_session_expire\n");
	};
	virtual void on_auth_failed(){
		fprintf(stderr,"on_auth_failed\n");
	};
	virtual void on_connecting(){
		fprintf(stderr,"on_connecting\n");
	};
	virtual void on_associating(){
		fprintf(stderr,"on_associating\n");
	};
	virtual void on_connected(){
		fprintf(stderr,"on_connected\n");
	};
	virtual void on_disconnected(){
		fprintf(stderr,"on_disconnected\n");
	};
	virtual void on_node_created(const char *path){
		fprintf(stderr,"on_node_created:%s\n",path);
	};
	virtual void on_node_value_changed(const char *path){
		fprintf(stderr,"on_node_value_changed:%s\n",path);
	};
	virtual void on_node_deleted(const char *path){
		fprintf(stderr,"on_node_deleted:%s\n",path);
	};
	virtual void on_child_changed(const char *path){
		fprintf(stderr,"on_child_changed:%s\n",path);
	};
	virtual void on_not_watching(const char *path){
		fprintf(stderr,"on_not_watching:%s\n",path);
	};

friend void active_watcher(zhandle_t *zh,int type,int state,const char *path,void* ctx);
};

/**
* @class ServiceWatcherAction Class for the watcher of 
*	the zookeeper,refresh connection pool once any node
*	is chaned,or there's anything wrong with the connection
*	to the zookeeper. 
*/
class ServiceWatcherAction:public WatcherAction
{
public:
    ServiceWatcherAction() {}
    ~ServiceWatcherAction() {}

public:
	virtual void on_connected();
	virtual void on_node_created(const char *path);
	virtual void on_node_value_changed(const char *path);
	virtual void on_node_deleted(const char *path);
	virtual void on_child_changed(const char *path);
};

/**
* @class RegServiceWatcherAction Class for the watcher of 
*	the zookeeper,used by the regist_service's zookeeper client.
*/
class RegServiceWatcherAction:public WatcherAction
{
public:
    RegServiceWatcherAction(vector<ServerCluster *> &server, WatcherAction *watcher_action = NULL)
    {
		std::map<string, boost::shared_ptr<ServerCluster> >::iterator itor;
		for (size_t i = 0; i < server.size(); ++i) {
			string server_node =  server[i]->get_server_node();
            //fprintf(stderr, "%s\n", server_node.c_str());
			itor = m_server.find(server_node);
			if (itor == m_server.end()) {
				m_server.insert(make_pair(server_node, boost::shared_ptr<ServerCluster>(server[i])));
			}
		}
		assert(m_server.size() > 0);
        m_watcher_action.reset(watcher_action);
	}

    explicit RegServiceWatcherAction(WatcherAction *watcher_action)
    {
        m_watcher_action.reset(watcher_action);
        assert(m_watcher_action != NULL);
    }

    RegServiceWatcherAction()
    {
        assert(!"default construtor invalid");
    }

	~RegServiceWatcherAction() {}

    virtual void on_connected()
    {
        std::map<string, boost::shared_ptr<ServerCluster> >::iterator itor = m_server.begin();
        while (itor != m_server.end())
        {
            itor->second->join();
            ++itor;
        }
        m_watcher_action != NULL ? m_watcher_action->on_connected() : WatcherAction::on_connected();
    }
    virtual void on_node_created(const char *path)
    {
		std::map<string, boost::shared_ptr<ServerCluster> >::iterator itor = m_server.find(string(path));
        if (itor == m_server.end()) {
            return m_watcher_action != NULL ? m_watcher_action->on_node_created(path) : WatcherAction::on_node_created(path);
        } else {
            bool exists;
            owner_->node_exists(path, exists);
            return WatcherAction::on_node_created(path);
        }
    }
    virtual void on_node_value_changed(const char *path)
	{
		std::map<string, boost::shared_ptr<ServerCluster> >::iterator itor = m_server.find(string(path));
        if (itor == m_server.end()) {
            return m_watcher_action != NULL ? m_watcher_action->on_node_value_changed(path) : WatcherAction::on_node_value_changed(path);
        } else {
			itor->second->join();
            return WatcherAction::on_node_value_changed(path);
		}
    };
    virtual void on_node_deleted(const char *path)
	{
		std::map<string, boost::shared_ptr<ServerCluster> >::iterator itor = m_server.find(string(path));
        if (itor == m_server.end()) {
            return m_watcher_action != NULL ? m_watcher_action->on_node_deleted(path) : WatcherAction::on_node_deleted(path);
        } else {
			itor->second->join();
            return WatcherAction::on_node_deleted(path);
		}
    };
    virtual void on_child_changed(const char *path)
	{
		std::map<string, boost::shared_ptr<ServerCluster> >::iterator itor = m_server.find(string(path));
        if (itor == m_server.end())
            return m_watcher_action != NULL ? m_watcher_action->on_child_changed(path) : WatcherAction::on_child_changed(path);
        else
            return WatcherAction::on_child_changed(path);
    };

private:
    boost::shared_ptr<WatcherAction> m_watcher_action;
	std::map<string, boost::shared_ptr<ServerCluster> > m_server;
};

inline void active_watcher(zhandle_t *zh,int type,int state,const char *path,void* ctx)
{
	if(zh==NULL || ctx==NULL){
		return;
	}

	WatcherAction *actionp=(WatcherAction*)ctx;

	if(type==ZOO_CREATED_EVENT){
		actionp->on_node_created(path);
	}else if(type==ZOO_DELETED_EVENT){
		actionp->on_node_deleted(path);
	}else if(type==ZOO_CHANGED_EVENT){
		actionp->on_node_value_changed(path);
	}else if(type==ZOO_CHILD_EVENT){
		actionp->on_child_changed(path);
	}else if(type==ZOO_SESSION_EVENT){
		if(state==ZOO_EXPIRED_SESSION_STATE){
			actionp->on_session_expired();
		}else if(state==ZOO_AUTH_FAILED_STATE){
			actionp->on_auth_failed();
		}else if(state==ZOO_CONNECTING_STATE){
			actionp->on_connecting();
		}else if(state==ZOO_ASSOCIATING_STATE){
			actionp->on_associating();
		}else if(state==ZOO_CONNECTED_STATE){
			actionp->on_connected();
		}
	}else if(type==ZOO_NOTWATCHING_EVENT){
		actionp->on_not_watching(path);
	}else{
		fprintf(stderr,"[ERROR] Invalid action type:%d\n",type);
	}
};
}
#endif
