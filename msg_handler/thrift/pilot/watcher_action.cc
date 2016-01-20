#include "watcher_action.h"
#include "update_task_manager.h"
#include "singleton_adapter.h"

namespace Pilot {

void ServiceWatcherAction::on_connected()
{
    SingletonAdapter<UpdateTaskManagerFactory>::instance()->notify_all(UpdateTaskManager::REFRESH_ALL,"");
	WatcherAction::on_connected();
}

void ServiceWatcherAction::on_node_created(const char *path)
{
	std::string spath(path);
	size_t found=spath.find_last_of("/");
	if(found!=std::string::npos){
		string key_path=spath.substr(0,found);
		if(key_path.size()>string(DEFAULT_NAMESPACE).size()){
			SingletonAdapter<UpdateTaskManagerFactory>::instance()->notify(key_path,UpdateTaskManager::NODE_CREATED,path);
		}
	}

	WatcherAction::on_node_created(path);
}

void ServiceWatcherAction::on_node_value_changed(const char *path)
{
	std::string spath(path);
	size_t found=spath.find_last_of("/");
	if(found!=std::string::npos){
		string key_path=spath.substr(0,found);
		if(key_path.size()>string(DEFAULT_NAMESPACE).size()){
			SingletonAdapter<UpdateTaskManagerFactory>::instance()->notify(key_path,UpdateTaskManager::NODE_VALUE_CHANGED,path);
		}
	}
	WatcherAction::on_node_value_changed(path);
}

void ServiceWatcherAction::on_node_deleted(const char *path)
{
	std::string spath(path);
	size_t found=spath.find_last_of("/");
	if(found!=std::string::npos){
		string key_path=spath.substr(0,found);
		if(key_path.size()>string(DEFAULT_NAMESPACE).size()){
			SingletonAdapter<UpdateTaskManagerFactory>::instance()->notify(key_path,UpdateTaskManager::NODE_DELETED,path);
		}
	}
	WatcherAction::on_node_deleted(path);
}

void ServiceWatcherAction::on_child_changed(const char *path)
{
	std::string spath(path);
	std::string key_path(spath);
	SingletonAdapter<UpdateTaskManagerFactory>::instance()->notify(key_path,UpdateTaskManager::CHILD_CHANGED,path);
	WatcherAction::on_child_changed(path);
}

}
