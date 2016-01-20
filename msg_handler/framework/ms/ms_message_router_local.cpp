#include "ms_message_router_local.h"
#include "ms_message_block_queue.h"
#include "ms_message.h"
#include "lib/thread.h"
#include "lib/posix_thread_factory.h"

#include <stdio.h>

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

class MsMessageRouterLocal::Impl: public util::Runnable
{
    public:
        explicit Impl(io::IoAppCluster* cluster);
        virtual ~Impl();

        void route(ms::IMessage* msg)
        {
            _queue.putList(msg);
        }

        virtual int run();

    private:
        io::IoAppCluster* _cluster;
        MsMessageBlockQueue _queue;
};

MsMessageRouterLocal::Impl::Impl(io::IoAppCluster* cluster):
    _cluster(cluster)
{
}

MsMessageRouterLocal::Impl::~Impl()
{
}

int MsMessageRouterLocal::Impl::run()
{
    LOG(INFO) << "local message router running ...\n";

#define IMIO_MAX_SIZE 1024
    //TODO,确保im_io线程不超过1024
    InternalMessageQueue sort[IMIO_MAX_SIZE];
    unsigned int io_count = _cluster->size();
    assert(io_count < IMIO_MAX_SIZE);

    while (true)
    {
        memset(sort, 0, io_count*sizeof(InternalMessageQueue));

        ms::IMessage* list = _queue.get();
        //合并相同io的message
        while (list)
        {
            ms::IMessage* msg = list;
            list = list->_next;

            msg->_next = 0;
            unsigned int io = msg->_io;
            assert(io < io_count);
            InternalMessageQueue& slot = sort[io];

            if (slot._tail)
                slot._tail->_next = msg;
            else
                slot._head = msg;
            slot._tail = msg;
            ++slot._count;
        }

        for (unsigned int i = 0; i < io_count; ++i)
        {
            InternalMessageQueue& slot = sort[i];
            if (slot._count > 0)
                _cluster->dispatchPushList(i, slot._head);
        }
    }

    LOG(INFO) << "local message router quit ...\n";

    return 0;
}

/*******************************************************************************
*******************************MsMessageRouterLocal*****************************
*******************************************************************************/
MsMessageRouterLocal::MsMessageRouterLocal(io::IoAppCluster* cluster):
    _core(new (std::nothrow) Impl(cluster))
{
    if (_core == 0)
    {
        LOG(INFO) << "memory error, abort\n";
        abort();
    }
}

MsMessageRouterLocal::~MsMessageRouterLocal()
{
    delete _core;
}

int MsMessageRouterLocal::run()
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

void MsMessageRouterLocal::route(void* msg)
{
    _core->route((ms::IMessage*)msg);
}

}
