#include "ms_message_router_remote.h"
#include "ms_message_block_queue.h"
#include "ms/ms_message.h"
#include "lib/block_queue.h"
#include "lib/thread.h"
#include "lib/posix_thread_factory.h"

#include <stdio.h>
#include <boost/unordered_map.hpp>

#include <glog/logging.h>

namespace ms
{

namespace
{

struct InternalMessageQueue
{
    ms::IMessage* _head;
    ms::IMessage* _tail;
    int _count;
};

}

class MsMessageRouterRemote::Impl: public util::Runnable
{
    public:
        explicit Impl(io::IoBackendCluster* cluster);
        virtual ~Impl();

        void route(ms::IMessage* msg)
        {
            _queue.putList(msg);
        }

        virtual int run();

    private:
        io::IoBackendCluster* _cluster;
        MsMessageBlockQueue _queue;
};

MsMessageRouterRemote::Impl::Impl(io::IoBackendCluster* cluster):
    _cluster(cluster)
{
}

MsMessageRouterRemote::Impl::~Impl()
{
}

int MsMessageRouterRemote::Impl::run()
{
    static boost::unordered_map<int64_t, InternalMessageQueue> map;
    boost::unordered_map<int64_t, InternalMessageQueue>::iterator it;

    LOG(INFO) << "remote message router running ...\n";

    InternalMessageQueue msg_item;;
    while (true)
    {
        map.clear();

        ms::IMessage* list = _queue.get();
        //合并相同出口的message
        while (list)
        {
            ms::IMessage* msg = list;
            list = list->_next;

            msg->_next = 0;
            boost::unordered_map<int64_t, InternalMessageQueue>::iterator it = map.find(msg->_conn);
            if (it != map.end())
            {
                InternalMessageQueue& target = it->second;
                target._tail->_next = msg;
                target._tail = msg;
                ++(target._count);
            }
            else
            {
                msg_item._tail = msg_item._head = msg;
                msg_item._count = 1;
                map[msg->_conn] = msg_item;
            }
        }

        it = map.begin();
        while (it != map.end())
        {
            _cluster->dispatchPushList(0, it->second._head);
            ++it;
        }
    }

    LOG(INFO) << "remote message router quit ...\n";

    return 0;
}

/*******************************************************************************
******************************MsMessageRouterRemote*****************************
*******************************************************************************/
MsMessageRouterRemote::MsMessageRouterRemote(io::IoBackendCluster* cluster):
    _core(new (std::nothrow) Impl(cluster))
{
    if (_core == 0)
    {
        LOG(INFO) << "memory error, abort\n";
        abort();
    }
}

MsMessageRouterRemote::~MsMessageRouterRemote()
{
    delete _core;
}

int MsMessageRouterRemote::run()
{
    util::PosixThreadFactory ptf;
    ptf.setPolicy(util::PosixThreadFactory::OTHER);
    ptf.setStackSize(10);
    ptf.setDetached(true);

    boost::shared_ptr<util::Runnable> core(_core);
    boost::shared_ptr<util::Thread> thread = ptf.newThread(core);
    thread->run();

    return 0;
}

void MsMessageRouterRemote::route(void* msg)
{
    _core->route((ms::IMessage*)msg);
}

}
