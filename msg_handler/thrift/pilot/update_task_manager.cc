#include "update_task_manager.h"
#include "stdio.h"
#include "singleton_adapter.h"
#include <assert.h>

UpdateTaskManager::UpdateTaskManager():update_task_pool_(NULL)
{

}

UpdateTaskManager::~UpdateTaskManager()
{
	close();
	if(update_task_pool_){
		delete []update_task_pool_;
		update_task_pool_=NULL;
	}
}

void UpdateTaskManager::open()
{
	update_task_pool_=new UpdateTaskNode[UPDATE_TASK_POOL_SIZE];
	for(int32_t i=0;i<UPDATE_TASK_POOL_SIZE;++i){
		update_wait_list_.put(update_task_pool_[i]);
	}

}

void UpdateTaskManager::close()
{
	update_wait_list_.flush();
	update_task_list_.flush();
}

bool UpdateTaskManager::put_update_task(UpdateTask *taskp)
{
    assert(taskp != NULL);
	UpdateTaskNode *node;
	if((node=update_wait_list_.try_get())!=NULL){
		node->data=taskp;
		update_task_list_.put(*node);
		return true;
	}else{
		fprintf(stderr,"UpdateTaskManager is over-loaded\n");
		return false;
	}
}
bool UpdateTaskManager::put_update_task(UpdateTaskType task_type,std::string task_arg)
{
    UpdateTask *taskp=new UpdateTask;
    taskp->task_type=task_type;
    taskp->task_arg=task_arg;
    taskp->task_timed_ = 0;
    if (task_type == TIMED_REFRESH_ALL) {
        struct timeval tv1;
        gettimeofday(&tv1, NULL);
        //目前支持3秒延时
        taskp->task_timed_ = tv1.tv_sec * 1000 + tv1.tv_usec / 1000 + 3000;
    }
    bool ret = put_update_task(taskp);
    if (ret == false)
        delete taskp;
    return ret;
}

UpdateTaskManager::UpdateTask *UpdateTaskManager::get_update_task()
{
	UpdateTaskNode *node;

	if((node=update_task_list_.get_from_head())!=NULL){
		UpdateTask *taskp=(UpdateTask*)node->data;
		update_wait_list_.put(*node);
		return taskp;
	}

	return NULL;
}

UpdateTaskManager *UpdateTaskManager::instance(string key)
{
	return SingletonAdapter<UpdateTaskManagerFactory>::instance()->get_update_task_manager(key);
}

UpdateTaskManagerFactory::~UpdateTaskManagerFactory()
{
    Pilot::MutexLock mlock(&lock_);
	update_task_manager_map_.erase(update_task_manager_map_.begin(),
		update_task_manager_map_.end());
}

UpdateTaskManager *UpdateTaskManagerFactory::get_update_task_manager(string key)
{
    Pilot::MutexLock mlock(&lock_);
    if(update_task_manager_map_.find(key)==update_task_manager_map_.end()){
        shared_ptr<UpdateTaskManager> update_task_manager(
                new UpdateTaskManager
                );
        update_task_manager->open();
        update_task_manager_map_.insert(make_pair(key,update_task_manager));
    }
	return update_task_manager_map_[key].get();
}

void UpdateTaskManagerFactory::notify_all(UpdateTaskManager::UpdateTaskType update_task,std::string task_arg)
{
    Pilot::MutexLock mlock(&lock_);
	map<string,shared_ptr<UpdateTaskManager> >::iterator iter;
	for(iter=update_task_manager_map_.begin();iter!=update_task_manager_map_.end();++iter){
		iter->second->put_update_task(update_task,task_arg);
	}
}

void UpdateTaskManagerFactory::notify(string key,UpdateTaskManager::UpdateTaskType update_task,string task_arg)
{
    Pilot::MutexLock mlock(&lock_);
	map<string,shared_ptr<UpdateTaskManager> >::iterator iter;
	iter=update_task_manager_map_.find(key);
	if(iter!=update_task_manager_map_.end()){
		iter->second->put_update_task(update_task,task_arg);
	}
}
