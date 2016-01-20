#include "io_backend_accept_service.h"
#include "io_backend_cluster.h"
#include "lib/util.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <glog/logging.h>

namespace io
{

IoBackendAcceptService::IoBackendAcceptService(int port, const char* ip, IoBackendCluster* cluster):
    _id(0),
    _cluster(cluster)
{
    _listen_socket = createListenSocket(port, ip);
}

IoBackendAcceptService::~IoBackendAcceptService()
{
    close(_listen_socket);
}

int IoBackendAcceptService::run(int fd)
{
    ++_id;
    LOG(INFO) << "new connection to backend im, socket: " << fd << ", conn: " << _id << std::endl;
    _cluster->dispatchConnection(fd, _id);
    return 0;
}

int IoBackendAcceptService::createListenSocket(int port, const char* ip)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        LOG(ERROR) << "create socket error, abort\n";
        abort();
    }

    if (util::setNonblocking(fd) == false)
    {
        LOG(ERROR) << "set socket nonblocking error\n";
        abort();
    }

    int flags = 1;
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
    if (ret != 0)
        LOG(ERROR) << "setsockopt so_reuseaddr error\n";

    ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
    if (ret != 0)
        LOG(ERROR) << "setsockopt so_reuseaddr error\n";

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    if (ip == 0 || ip[0] == '\0')
        sin.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        sin.sin_addr.s_addr = inet_addr(ip);

    ret = bind(fd, (struct sockaddr*)&sin, sizeof(sin));
    if (ret == -1)
    {
        LOG(ERROR) << "bind socket error[port:" << port << "], abort\n";
        abort();
    }

    ret = listen(fd, 128);
    if (ret == -1)
    {
        LOG(ERROR) << "listen socket error, abort\n";
        abort();
    }

    return fd;
}

}
