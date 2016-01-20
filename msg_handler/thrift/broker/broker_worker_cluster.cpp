#include "broker_worker_cluster.h"
#include "broker_message_core.h"

#include <stdlib.h>
#include <assert.h>

#include <glog/logging.h>

namespace im
{

BrokerWorkerCluster::BrokerWorkerCluster(int cluster_size, ms::MsMessageProcessor* processor):
    _cluster_size(cluster_size),
    _worker_core(new (std::nothrow) BrokerMessageWorkerCore(processor, &_queue))
{
    assert(_cluster_size > 0);
    assert(_worker_core);
}

BrokerWorkerCluster::~BrokerWorkerCluster()
{
}

int BrokerWorkerCluster::run()
{
    if (_cluster_size <= 0)
        return -1;

    _cluster = new (std::nothrow) ms::MsWorker*[_cluster_size];
    if (_cluster == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    ms::MsWorker* w = 0;
    for (int i = 0; i < _cluster_size; ++i)
    {
        w = new (std::nothrow) ms::MsWorker(i, _worker_core);
        if (w == 0)
        {
            LOG(ERROR) << "memory error, abort\n";
            abort();
        }
        _cluster[i] = w;

        w->run();
    }

    return 0;
}

}

