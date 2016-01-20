#ifndef __IO_BACKEND_ACCEPT_SERVICE_H__
#define __IO_BACKEND_ACCEPT_SERVICE_H__

#include "accept_service.h"

#include "lib/queue.h"

namespace io
{

class IoBackendCluster;
class IoBackendAcceptService : public AcceptService
{
    public:
        IoBackendAcceptService(int port, const char* ip, IoBackendCluster* cluster);
        virtual ~IoBackendAcceptService();

        virtual int run(int fd);

    private:
        int createListenSocket(int port, const char* ip);

    private:
        int64_t _id;
        IoBackendCluster* _cluster;
};

}

#endif
