#include "io_backend_cluster.h"
#include "io_backend.h"
#include "lib/queue.h"

#include <stdio.h>
#include <stdlib.h>

#include <glog/logging.h>

namespace io
{

IoBackendCluster::IoBackendCluster(ms::MsWorkerCluster* workers):
    _workers(workers),
    _core_io(0)
{
}

IoBackendCluster::~IoBackendCluster()
{
    delete _core_io;
}

int IoBackendCluster::run()
{
    _core_io = new (std::nothrow) IoBackend(0, &_sock_queue, &_msg_queue, _workers);
    if (_core_io == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    _core_io->setReadTimeout(0);
    _core_io->setWriteTimeout(0);
    _core_io->run();

    return 0;
}

int IoBackendCluster::dispatchConnection(int fd, int64_t id)
{
    _sock_queue.put(fd, id);
    _core_io->newNotify();
    return 0;
}

int IoBackendCluster::dispatchPushList(int io, ms::IMessage* msg)
{
    (void)io;
    _msg_queue.put(msg);
    _core_io->pushNotify();
    return 0;
}

}
