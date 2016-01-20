#ifndef __BROKER_WORKER_CLUSTER_H__
#define __BROKER_WORKER_CLUSTER_H__

#include "framework/ms/ms_message_block_queue.h"
#include "framework/ms/ms_message.h"
#include "framework/ms/ms_worker.h"
#include "framework/ms/ms_worker_cluster.h"
#include "framework/ms/ms_message_processor.h"

namespace im
{

class BrokerMessageWorkerCore;
class BrokerWorkerCluster : public ms::MsWorkerCluster
{
    public:
        BrokerWorkerCluster(int cluster_size, ms::MsMessageProcessor* processor);
        virtual ~BrokerWorkerCluster();

        virtual void schedule(ms::IMessage* msg) { _queue.put(msg); }
        int run();

    private:
        int _cluster_size;
        ms::MsMessageBlockQueue _queue;
        ms::MsWorker** _cluster;
        BrokerMessageWorkerCore* _worker_core;
};

}

#endif
