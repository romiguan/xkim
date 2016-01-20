#ifndef __MS_WORKER_CLUSTER_H__
#define __MS_WORKER_CLUSTER_H__

namespace ms
{

class MsWorkerCluster
{
    public:
        MsWorkerCluster() {}
        virtual ~MsWorkerCluster() {}

        virtual void schedule(IMessage* msg) = 0;
};

}

#endif
