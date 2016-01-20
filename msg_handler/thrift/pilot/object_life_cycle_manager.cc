#include "object_life_cycle_manager.h"

Cleanup::Cleanup()
{

}

Cleanup::~Cleanup()
{

}

void Cleanup::cleanup()
{
	delete this;
}

ObjectLifeCycleManager::ObjectLifeCycleManager()
{

}

ObjectLifeCycleManager::~ObjectLifeCycleManager()
{
   Pilot::MutexLock mlock(&lock_);
	vector<object_t>::reverse_iterator riter;
	riter=ObjectLifeCycleManager::object_vec_.rbegin();
	while(riter!=object_vec_.rend()){
		riter->cleanup_hook(riter->objectp,riter->paramp);
		++riter;
	}
}

vector<ObjectLifeCycleManager::object_t> ObjectLifeCycleManager::object_vec_;
Pilot::Lock ObjectLifeCycleManager::lock_;
