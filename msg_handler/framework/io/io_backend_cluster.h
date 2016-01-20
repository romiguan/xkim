#ifndef __IO_BACKEND_CLUSTER_H__
#define __IO_BACKEND_CLUSTER_H__

#include "io_backend.h"
#include "ms/ms_message.h"

namespace io
{

class IoBackend;
class IoBackendCluster
{
    public:
        explicit IoBackendCluster(ms::MsWorkerCluster* workers);
        ~IoBackendCluster();

        int run();
        int dispatchConnection(int fd, int64_t id);
        int dispatchPushList(int io, ms::IMessage* msg);

    private:
        BackendSocketQueue _sock_queue;
        BackendMessageQueue _msg_queue;
        ms::MsWorkerCluster* _workers;
        IoBackend* _core_io;
};

}

#endif
