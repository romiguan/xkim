#ifndef __IM_APP_WORKER_CLUSTER_H__
#define __IM_APP_WORKER_CLUSTER_H__

#include "framework/ms/ms_message_priority_queue.h"
#include "framework/ms/ms_message.h"
#include "framework/ms/ms_worker.h"
#include "framework/ms/ms_worker_cluster.h"
#include "framework/ms/ms_message_processor.h"

namespace im
{

class ImAppMessageWorkerCore;
class ImAppWorkerCluster : public ms::MsWorkerCluster
{
    public:
        ImAppWorkerCluster(int cluster_size, ms::MsMessageProcessor* processor);
        virtual ~ImAppWorkerCluster();

        virtual void schedule(ms::IMessage* msg) { _pri_queue.put(msg, msg->_priority); }
        int run();

    private:
        int _cluster_size;
        ms::MsMessagePriorityQueue _pri_queue;
        ms::MsWorker** _cluster;
        ImAppMessageWorkerCore* _worker_core;
};

}

#endif
