#include "im_app_worker_cluster.h"
#include "im_app_message_core.h"

#include <stdlib.h>
#include <assert.h>

#include <glog/logging.h>

namespace im
{

ImAppWorkerCluster::ImAppWorkerCluster(int cluster_size, ms::MsMessageProcessor* processor):
    _cluster_size(cluster_size),
    _cluster(0)
{
    assert(_cluster_size > 0);
    _pri_queue.init(2);
    _worker_core = new (std::nothrow) ImAppMessageWorkerCore(processor, &_pri_queue);
    assert(_worker_core);
}

ImAppWorkerCluster::~ImAppWorkerCluster()
{
}

int ImAppWorkerCluster::run()
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

