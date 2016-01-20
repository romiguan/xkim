#include "io_accept.h"
#include "accept_service.h"

#include "lib/thread.h"
#include "lib/posix_thread_factory.h"

#include <event.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <glog/logging.h>

#define SOCKET_BUFFER_SIZE 10240

namespace io
{

namespace
{

inline bool setNonblocking(int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        return false;
    return true;
}

}

void _callback(int fd, short which, void* v);
int _accept(int fd);

class IoAccept::Impl : public util::Runnable
{
    public:
        Impl();
        ~Impl();

        int addService(AcceptService* s);
        virtual int run();
        //TODO
        virtual int stop();

    private:
        static const int EVENT_MAX = 8;
        struct event_base* _event_base;
        struct event _event[EVENT_MAX];
        AcceptService* _service[EVENT_MAX];
        int _service_size;
        int _stop;
};

IoAccept::Impl::Impl():
    _event_base(event_base_new()),
    _service_size(0)
{
    if (_event_base == 0)
    {
        LOG(ERROR) << "IoAccept construct error, abort\n";
        abort();
    }
}

IoAccept::Impl::~Impl()
{
    for (int i = 0; i < _service_size; ++i)
    {
        delete _service[i];
    }

    if (_event_base)
        event_base_free(_event_base);

    _event_base = 0;
    _service_size = 0;
}

int IoAccept::Impl::addService(AcceptService* s)
{
    if (_service_size >= EVENT_MAX || s == 0)
        return -1;

    _service[_service_size++] = s;
    s->_io = _event_base;

    return 0;
}

int IoAccept::Impl::run()
{
    if (_service_size == 0)
        return 0;

    LOG(INFO) << "accept io running ...\n";

    AcceptService* s = 0;
    struct event* e = 0;
    for (int i = 0; i < _service_size; ++i)
    {
        s = _service[i];
        e = &_event[i];
        event_set(e, s->_listen_socket, EV_READ|EV_PERSIST, _callback, s);
        event_base_set(_event_base, e); 
        event_add(e, 0); 
    }

    event_base_loop(_event_base, 0);

    LOG(INFO) << "accept io quit ...\n";

    return 0;
}

//TODO
int IoAccept::Impl::stop()
{
    return -1;
}

int _accept(int fd)
{
    int new_fd = -1;
    while (true)
    {
        struct sockaddr_in sock;
        socklen_t sockLen = sizeof(sock);
        new_fd = ::accept(fd, (struct sockaddr*)&sock, &sockLen);
        if (new_fd == -1)
        {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }

        break;
    }

    if (setNonblocking(new_fd) == false)
    {
        LOG(ERROR) << "set socket nonblocking error\n";
        ::close(new_fd);
        return -1;
    }

    int send_buffer_size = SOCKET_BUFFER_SIZE;
    setsockopt(new_fd, SOL_SOCKET, SO_SNDBUF, (void*)&send_buffer_size, sizeof(send_buffer_size));

    int flags = 1;
    int ret = setsockopt(new_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
    ret = setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
    if (ret != 0)
        LOG(ERROR) << "setsockopt tcp_nodelay error\n";

    struct linger ling = {0, 0};
    setsockopt(new_fd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
    if (ret != 0)
        LOG(ERROR) << "setsockopt tcp_linger error\n";

    return new_fd;
}

void _callback(int fd, short which, void* v)
{
    (void)which;
    int new_fd = _accept(fd);
    if (new_fd != -1)
    {
        AcceptService* service = (AcceptService*)v;
        service->run(new_fd);
    }
}

/*
 * ##############################################################
 * ################### IoAccept #################################
 * ##############################################################
 */
IoAccept::IoAccept():
    _core(new (std::nothrow) IoAccept::Impl())
{
    if (_core == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }
}

IoAccept::~IoAccept()
{
    delete _core;
}

int IoAccept::addService(AcceptService* s)
{
    return _core->addService(s);
}

boost::shared_ptr<util::Thread> io_thread;

int IoAccept::run()
{
    util::PosixThreadFactory ptf;
    ptf.setPolicy(util::PosixThreadFactory::OTHER);
    ptf.setStackSize(10);
    ptf.setDetached(false);

    boost::shared_ptr<util::Runnable> io(_core);
    io_thread = ptf.newThread(io);
    io_thread->run();

    return 0;
}

int IoAccept::stop()
{
    return _core->stop();
}

int IoAccept::join()
{
    return io_thread->join();
}

}
