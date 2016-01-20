#include "ms_message_collector.h"
#include "ms_message_block_queue.h"
#include "ms_message.h"
#include "io/io_channel.h"

#include "lib/posix_thread_factory.h"
#include "lib/thread.h"

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

class MsMessageCollector::Impl : public util::Runnable
{
    public:
        Impl() {}
        virtual ~Impl() {}

        virtual int run();
        virtual int stop();

        void setCollector(MsMessageCollectorCore* c)
        {
            _core = c;
        }

        void collect(IMessage* msg)
        {
            _queue.put(msg);
        }

        void collectList(IMessage* msg)
        {
            _queue.putList(msg);
        }

    private:
        MsMessageBlockQueue _queue;
        MsMessageCollectorCore* _core;
};

int MsMessageCollector::Impl::run()
{
    static boost::unordered_map<int, InternalMessageQueue> _map;
    boost::unordered_map<int, InternalMessageQueue>::iterator it;

    if (_core == 0)
    {
        LOG(INFO) << "none collector, abort\n";
        abort();
    }

    LOG(INFO) << "message collect worker running ...\n";

    InternalMessageQueue msg_item;;
    while (true)
    {
        _map.clear();

        ms::IMessage* list = _queue.get();
        //合并相同uid的message
        while (list)
        {
            ms::IMessage* msg = list;
            list = list->_next;

            msg->_next = 0;
            boost::unordered_map<UID, InternalMessageQueue>::iterator it = _map.find(msg->_uid);
            if (it != _map.end())
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
                _map[msg->_uid] = msg_item;
            }
        }

        it = _map.begin();
        while (it != _map.end())
        {
            _core->collect(it->second._head);
            ++it;
        }
    }

    LOG(INFO) << "message collect worker quit ...\n";

    return 0;
}

int MsMessageCollector::Impl::stop()
{
    return -1;
}

/*******************************************************************************
****************************** MsMessageCollector ******************************
*******************************************************************************/
MsMessageCollector::MsMessageCollector():
    _core(new (std::nothrow) Impl())
{
    if (_core == 0)
    {
        LOG(INFO) << "memory error, abort\n";
        abort();
    }
}

MsMessageCollector::~MsMessageCollector()
{
    delete _core;
}

int MsMessageCollector::run()
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

void MsMessageCollector::setCollector(MsMessageCollectorCore* c)
{
    _core->setCollector(c);
}

void MsMessageCollector::collect(IMessage* msg)
{
    _core->collect(msg);
}

void MsMessageCollector::collectList(IMessage* msg)
{
    _core->collectList(msg);
}

}
