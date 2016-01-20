#ifndef __SINGLETON_ADAPTER_H__
#define __SINGLETON_ADAPTER_H__

#include "lock.h"
#include "object_life_cycle_manager.h"

template<typename TYPE>
class SingletonAdapter:public Cleanup
{
public:
	static TYPE *instance(){
		if(flag_==0){
            Pilot::MutexLock mlock(&lock_);
			if(singleton_==0){
				singleton_=new SingletonAdapter<TYPE>;
                ObjectLifeCycleManager::at_exit(singleton_);
			}
			flag_=1;
		}
		return &singleton_->instance_;
	};

protected:
	TYPE instance_;
	static SingletonAdapter<TYPE> * volatile singleton_;
	static Pilot::Lock lock_;
	static volatile int flag_;

private:
	//SingletonAdapter(){};
	//~SingletonAdapter(){};
	//SingletonAdapter(const SingletonAdapter &);
	//SingletonAdapter &operator=(const SingletonAdapter &);
};

template<typename TYPE>
SingletonAdapter<TYPE> *volatile SingletonAdapter<TYPE>::singleton_=0;
template<typename TYPE>
Pilot::Lock SingletonAdapter<TYPE>::lock_;
template<typename TYPE>
volatile int SingletonAdapter<TYPE>::flag_=0;

#define DECLARE_SINGLETON_CLASS(type) \
	friend class shared_ptr<type>;\
	friend class SingletonAdapter<type>

#endif //__SINGLETON_ADAPTER_H__
