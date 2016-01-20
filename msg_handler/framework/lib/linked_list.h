#ifndef __UTIL_LINKED_LIST_H__
#define __UTIL_LINKED_LIST_H__

#include <stdlib.h>

namespace util
{

struct ListNodeLinkType
{
    ListNodeLinkType* _next;
    ListNodeLinkType* _prev;
};

//双向循环链表
template <typename T, ListNodeLinkType T::* ListNode>
class LinkedListType
{
    public:
        LinkedListType()
        {
            _head._next = _head._prev = &_head;
        }

        bool empty() const
        {
            return _head._next == &_head;
        }

        ListNodeLinkType& head() { return _head; }

        T* entry(ListNodeLinkType& node) const
        {
            return &node == &_head ? NULL : (T*)((char*)&node-(char*)_node_offset);
        }

        void add(T& node)
        {
            _head._next->_prev = &(node.*ListNode);
            (node.*ListNode)._next = _head._next;
            (node.*ListNode)._prev = &_head;
            _head._next = &(node.*ListNode);
        }

        void addLast(T& node)
        {
            _head._prev->_next = &(node.*ListNode);
            (node.*ListNode)._prev = _head._prev;
            (node.*ListNode)._next = &_head;
            _head._prev = &(node.*ListNode);
        }

        T* first() const
        {
            return empty() ? NULL : (T*)((char*)(_head._next)-(char*)_node_offset);
        }

        T* last() const
        {
            return empty() ? NULL : (T*)((char*)(_head._prev)-(char*)_node_offset);
        }

        T* _next(T& node) const
        {
            return (node.*ListNode)._next == &_head ? NULL : (T*)((char*)(node.*ListNode)._next-(char*)_node_offset);
        }

        T* _prev(T& node) const
        {
            return (node.*ListNode)._prev == &_head ? NULL : (T*)((char*)(node.*ListNode)._prev-(char*)_node_offset);
        }

        T* _next(ListNodeLinkType& node) const
        {
            return node._next == &_head ? NULL : (T*)((char*)node._next-(char*)_node_offset);
        }

        T* _prev(ListNodeLinkType& node) const
        {
            return node._prev == &_head ? NULL : (T*)((char*)node._prev-(char*)_node_offset);
        }

        static void addPrev(T& node, T& cur)
        {
            (cur.*ListNode)._prev->_next = &(node.*ListNode);
            (node.*ListNode)._prev = (cur.*ListNode)._prev;
            (node.*ListNode)._next = &(cur.*ListNode);
            (cur.*ListNode)._prev = &(node.*ListNode);
        }

        static void addNext(T& node, T& cur)
        {
            (cur.*ListNode)._next->_prev = &(node.*ListNode);
            (node.*ListNode)._next = (cur.*ListNode)._next;
            (node.*ListNode)._prev = &(cur.*ListNode);
            (cur.*ListNode)._next = &(node.*ListNode);
        }

        static void remove(T& node)
        {
            (node.*ListNode)._next->_prev = (node.*ListNode)._prev;
            (node.*ListNode)._prev->_next = (node.*ListNode)._next;
        }

    protected:
        static ListNodeLinkType const * const _node_offset;
        ListNodeLinkType _head;
};

template <typename T, ListNodeLinkType T::* ListNode>
ListNodeLinkType const * const LinkedListType<T,ListNode>::_node_offset = &(((T *)0)->*ListNode);

}

#endif

