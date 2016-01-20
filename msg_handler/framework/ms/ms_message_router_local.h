#ifndef __MS_MESSAGE_ROUTER_LOCAL_H__
#define __MS_MESSAGE_ROUTER_LOCAL_H__

#include "io/io_app_cluster.h"

namespace ms
{

class MsMessageRouterLocal
{
    public:
        explicit MsMessageRouterLocal(io::IoAppCluster* cluster);
        ~MsMessageRouterLocal();

        int run();
        void route(void* msg);

    private:
        class Impl;
        Impl* _core;
};

}

#endif
