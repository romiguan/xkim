#include "io_backend_connection.h"
#include "ms/ms_message.h"

#include <glog/logging.h>

namespace io
{

IoBackendConnection::IoBackendConnection():
    IoConnection(),
    _workers(0)
{
}

IoBackendConnection::IoBackendConnection(int fd):
    IoConnection(fd),
    _workers(0)
{
}

IoBackendConnection::~IoBackendConnection()
{
}

int IoBackendConnection::whenReceivedFrame(char* buf, int size)
{
    if (_workers)
    {
        ms::IMessage* m = ms::createIMessage(buf, size);
        m->_io = _io;
        m->_conn = _id;
        m->_type = ms::BACKEND;

        _workers->schedule(m);
        return 0;
    }

    return -1;
}

void IoBackendConnection::initialize(int fd,
        int io,
        int64_t id,
        void* owner,
        struct event_base* event_base,
        ms::MsWorkerCluster* workers)
{
    IoConnection::initialize(fd, io, id, owner, event_base);
    _workers = workers;
}

}
