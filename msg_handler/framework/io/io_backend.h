#ifndef __IO_BACKEND_H__
#define __IO_BACKEND_H__

#include "lib/queue.h"
#include "ms/ms_message_queue.h"
#include "ms/ms_worker_cluster.h"

namespace io
{

typedef concurrency::Queue BackendSocketQueue;
typedef ms::MsMessageQueue BackendMessageQueue;

class IoBackend
{
    public:
        IoBackend(int id,
                BackendSocketQueue* sock_queue,
                BackendMessageQueue* msg_queue,
                ms::MsWorkerCluster* workers);
        virtual ~IoBackend();

        int run();
        int stop();

        void newNotify();
        void pushNotify();
        void setReadTimeout(int timeout);
        void setWriteTimeout(int timeout);

        int id() const;

    private:
        class Impl;
        Impl* _core;
};

}

#endif
