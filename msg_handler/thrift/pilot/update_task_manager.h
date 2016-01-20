#ifndef __UPDATE_TASK_MANAGER_H__
#define __UPDATE_TASK_MANAGER_H__

#include "defines.h"
#include "thread_base.h"
#include "wait_list.h"
#include "lock.h"
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
using namespace std;
using namespace boost;

class UpdateTaskManager
{
public:
	enum UpdateTaskType{
		REFRESH_ALL = 1,
        TIMED_REFRESH_ALL = 2,
		SESSION_EXPIRED = 4,
		NODE_CREATED = 8,
		NODE_VALUE_CHANGED = 16,
		NODE_DELETED = 32,
		CHILD_CHANGED = 64,
		EXIT = 128
	};
	struct UpdateTask
	{
		UpdateTaskManager::UpdateTaskType task_type;
		std::string task_arg;
        uint32_t task_timed_;  //定时调度,绝对时间,毫秒
	};
	struct UpdateTaskNode
	{
		linked_list_node_t list_node;
		void *data;
	};

public:
	static UpdateTaskManager *instance(string key);

public:
	UpdateTaskManager();
	~UpdateTaskManager();

public:
	/**
	* Open the task queue
	*/
	void open();
	/**
	* Close the task queue
	*/
	void close();
	/**
	* Put an update task into the task queue.
	*
	* @param type Type of the update task,see enum UpdateTaskType.
	* @param arg Argument of the update task.Usually it's the path
	*	of the node that's changed.
	* @return true if put one task into queue successfully;
	*	false if NOT,usually because the queue is full.
    */
    bool put_update_task(UpdateTaskType task_type,std::string task_arg);
    bool put_update_task(UpdateTask *taskp);
	/**
	* Get an update task from the task queue.
	*
	* @return A pointer of one UpdateTask instance.
	*/
	UpdateTask *get_update_task();

protected:
	wait_list_t<UpdateTaskNode,&UpdateTaskNode::list_node> update_task_list_;
	wait_list_t<UpdateTaskNode,&UpdateTaskNode::list_node> update_wait_list_;
	UpdateTaskNode *update_task_pool_;

};

class UpdateTaskManagerFactory
{
public:
	~UpdateTaskManagerFactory();

public:
	UpdateTaskManager *get_update_task_manager(string key);
	void notify_all(UpdateTaskManager::UpdateTaskType update_task,string task_arg);
	void notify(string key,UpdateTaskManager::UpdateTaskType update_task,string task_arg);

private:
	map<string,shared_ptr<UpdateTaskManager> > update_task_manager_map_;
    Pilot::Lock lock_;
};

#endif
