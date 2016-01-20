#ifndef __UNSAFE_WAIT_LIST_H__
#define __UNSAFE_WAIT_LIST_H__

#include <pthread.h>
#include "linked_list.h"

template <typename T,linked_list_node_t T::*list_node>
class unsafe_wait_list_t:public linked_list_t<T,list_node>
{
public:
    unsafe_wait_list_t():_alive(1),_num(0)
    {
    }

    ~unsafe_wait_list_t()
    {
    }

    int len()
    {
        return _num;
    }

    void put(T &node)
    {
        if(_alive){
            this->add(node);
            ++_num;
        }
    }

    T* try_get()
    {
        T *ret;

        if(_alive && !linked_list_t<T,list_node>::is_empty()){
            ret=this->entry(*linked_list_t<T,list_node>::_head.prev);
            this->del(*ret);
            --_num;
        }else{
            ret=NULL;
        }

        return ret;
    }

    T* try_get_from_head()
    {
        T *ret;

        if(_alive && !linked_list_t<T,list_node>::is_empty()){
            ret=this->entry(*linked_list_t<T,list_node>::_head.next);
            this->del(*ret);
            --_num;
        }else{
            ret=NULL;
        }

        return ret;
    }

    void flush()
    {
        _alive=0;
    }

protected:
    int _alive;
    int _num;
};

#endif // __UNSAFE_WAIT_LIST_H__ 

