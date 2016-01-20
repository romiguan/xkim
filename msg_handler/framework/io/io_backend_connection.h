#ifndef __IO_BACKEND_CONNECTION_H__
#define __IO_BACKEND_CONNECTION_H__

#include "io_connection.h"
#include "connection_cache.h"
#include "connection_register.h"
#include "ms/ms_worker_cluster.h"

#include <stdlib.h>
#include <event.h>

namespace io
{

class IoBackendConnection : public IoConnection
{
    public:
        IoBackendConnection();
        explicit IoBackendConnection(int fd);
        virtual ~IoBackendConnection();

        void initialize(int fd,
                int io,
                int64_t id,
                void* owner,
                struct event_base* event_base,
                ms::MsWorkerCluster* workers);

    protected:
        virtual int whenReceivedFrame(char* buf, int size);

    private:
        ms::MsWorkerCluster* _workers;
};

}

#endif
