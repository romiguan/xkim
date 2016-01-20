#ifndef __IO_APP_ACCEPT_SERVICE_H__
#define __IO_APP_ACCEPT_SERVICE_H__

#include "accept_service.h"

#include "lib/queue.h"

namespace io
{

class IoAppCluster;
class IoAppAcceptService : public AcceptService
{
    public:
        IoAppAcceptService(int port, const char* ip, IoAppCluster* cluster);
        virtual ~IoAppAcceptService();

        virtual int run(int fd);

    private:
        int createListenSocket(int port, const char* ip);

    private:
        uint64_t _id;
        IoAppCluster* _cluster;
};

}

#endif
