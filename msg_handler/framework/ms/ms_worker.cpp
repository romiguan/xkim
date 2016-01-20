#include "ms_worker.h"
#include "ms_worker_core.h"
#include "ms_message.h"

#include "lib/posix_thread_factory.h"
#include "lib/thread.h"

#include <stdio.h>

#include <glog/logging.h>

namespace ms
{

class MsWorker::Impl : public util::Runnable
{
    public:
        Impl(int id, MsWorkerCore* core);
        virtual ~Impl();

        virtual int run();
        virtual int stop();

    private:
        int _id;
        MsWorkerCore* _core;
};

MsWorker::Impl::Impl(int id, MsWorkerCore* core):
    _id(id),
    _core(core)
{
}

MsWorker::Impl::~Impl()
{
}

int MsWorker::Impl::run()
{
    LOG(INFO) << "im worker(" << _id << ") running ...\n";

    _core->run();

    LOG(INFO) << "im worker(" << _id << ") quit ...\n";

    return 0;
}

int MsWorker::Impl::stop()
{
    return -1;
}

/*******************************************************************************
*******************************MsWorker*******************************************
*******************************************************************************/
MsWorker::MsWorker(int id, MsWorkerCore* core):
    _impl(new (std::nothrow) Impl(id, core))
{
    if (_impl == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }
}

MsWorker::~MsWorker()
{
    delete _impl;
}

int MsWorker::run()
{
    util::PosixThreadFactory ptf;
    ptf.setPolicy(util::PosixThreadFactory::OTHER);
    ptf.setStackSize(10);
    ptf.setDetached(true);

    boost::shared_ptr<util::Runnable> core(_impl);
    boost::shared_ptr<util::Thread> thread = ptf.newThread(core);
    thread->run();

    return 0;
}

int MsWorker::stop()
{
    return _impl->stop();
}

}

