#ifndef __MS_MESSAGE_ROUTER_REMOTE_H__
#define __MS_MESSAGE_ROUTER_REMOTE_H__

#include "io/io_backend_cluster.h"

namespace ms
{

class MsMessageRouterRemote
{
    public:
        explicit MsMessageRouterRemote(io::IoBackendCluster* cluster);
        ~MsMessageRouterRemote();

        int run();
        void route(void* msg);

    private:
        class Impl;
        Impl* _core;
};

}

#endif
