#ifndef __IM_AUTH_WORKER_CLUSTER_H__
#define __IM_AUTH_WORKER_CLUSTER_H__

#include "framework/ms/ms_message_block_queue.h"
#include "framework/ms/ms_message.h"
#include "framework/ms/ms_worker.h"
#include "framework/ms/ms_worker_cluster.h"
#include "framework/ms/ms_message_processor.h"

namespace im
{

class ImAuthMessageWorkerCore;
class ImAuthWorkerCluster : public ms::MsWorkerCluster
{
    public:
        ImAuthWorkerCluster(int cluster_size, ms::MsMessageProcessor* processor);
        virtual ~ImAuthWorkerCluster();

        virtual void schedule(ms::IMessage* msg) { _queue.put(msg); }
        int run();

    private:
        int _cluster_size;
        ms::MsMessageBlockQueue _queue;
        ms::MsWorker** _cluster;
        ImAuthMessageWorkerCore* _worker_core;
};

}

#endif
