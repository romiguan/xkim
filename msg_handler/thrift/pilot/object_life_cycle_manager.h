#ifndef __OBJECT_LIFE_CYCLE_MANAGER_H__
#define __OBJECT_LIFE_CYCLE_MANAGER_H__

#include <vector>
#include "lock.h"
using namespace std;

class Cleanup
{
public:
	Cleanup();
	virtual ~Cleanup();
	virtual void cleanup();
};

typedef void (*CLEANUP_FUNC)(void *objectp,void *paramp);
inline void default_cleanup_hook(void *objectp,void *paramp)
{
    (void)paramp;
	((Cleanup *)objectp)->cleanup();
};

class ObjectLifeCycleManager
{
private:
	typedef struct{
		void			*objectp;
		CLEANUP_FUNC 	cleanup_hook;
		void			*paramp;
	}object_t;
public:
	ObjectLifeCycleManager();
	~ObjectLifeCycleManager();
	static void at_exit(void *objectp,CLEANUP_FUNC cleanup_hook=&default_cleanup_hook,void *paramp=0){
		object_t obj;
		obj.objectp=objectp;
		obj.cleanup_hook=cleanup_hook;
		obj.paramp=paramp;
        Pilot::MutexLock mlock(&lock_);
		object_vec_.push_back(obj);
	};
private:
	static vector<object_t> object_vec_;
	static Pilot::Lock lock_;
};

#endif
