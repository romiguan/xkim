#include "io_backend.h"
#include "io_backend_connection.h"
#include "ms/ms_message.h"
#include "lib/util.h"
#include "lib/thread.h"
#include "lib/posix_thread_factory.h"
#include "connection_register.h"

#include <event.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <boost/unordered_map.hpp>
#include <glog/logging.h>

namespace io
{

namespace
{

struct InternalMessageQueue
{
    ms::IMessage* _head;
    ms::IMessage* _tail;
    int _count;
};

}

class IoBackend::Impl : public util::Runnable
{
    public:
        Impl(IoBackend* owner,
                int id,
                BackendSocketQueue* sock_queue,
                BackendMessageQueue* msg_queue,
                ms::MsWorkerCluster* workers);

        virtual ~Impl();

        virtual int run();
        virtual int stop();

        void setReadTimeout(int timeout) { _rtimeout = timeout; }
        void setWriteTimeout(int timeout) { _wtimeout = timeout; }
        void newNotify()
        {
            int ret = ::write(_new_wnotify, "", 1);
            (void)ret;
        }
        void pushNotify()
        {
            int ret = ::write(_push_wnotify, "", 1);
            (void)ret;
        }

        int id() const { return _id; }

    private:
        static void connectionHandle(int fd, short which, void* v);
        static void pushHandle(int fd, short which, void* v);

        static void outstream_destroy(IoConnection* conn);
        static void destroy(IoConnection* conn);

    private:
        int createSocket(uint64_t key);
        void createEvent();
        void newConnection(int fd);
        void pushMessage(ms::IMessage* msg);
        void push(int fd);

    private:
        IoBackend* _owner;
        int _id;
        int _rtimeout;
        int _wtimeout;
        int _new_rnotify;
        int _new_wnotify;
        int _push_rnotify;
        int _push_wnotify;
        struct event_base* _event_base;
        struct event _new_event;
        struct event _push_event;
        BackendSocketQueue* _sock_queue;
        BackendMessageQueue* _msg_queue;
        ms::MsWorkerCluster* _workers;
        ConnectionCache<IoBackendConnection> _cache;
        ConnectionRegister<IoBackendConnection> _online_register;
};

IoBackend::Impl::Impl(IoBackend* owner,
        int id,
        BackendSocketQueue* sock_queue,
        BackendMessageQueue* msg_queue,
        ms::MsWorkerCluster* workers):
    _owner(owner),
    _id(id),
    _rtimeout(0),
    _wtimeout(0),
    _event_base(event_base_new()),
    _sock_queue(sock_queue),
    _msg_queue(msg_queue),
    _workers(workers),
    _cache(id)
{
    if (_event_base == 0)
    {
        LOG(ERROR) << "IoBackend construct error, abort\n";
        abort();
    }
}

IoBackend::Impl::~Impl()
{
    if (_event_base)
        event_base_free(_event_base);
}

void IoBackend::Impl::destroy(IoConnection* conn)
{
    Impl* io = (Impl*)(conn->_owner);
    IoBackendConnection* backend_conn = (IoBackendConnection*)conn;
    io->_cache.recycle(backend_conn);
}

void IoBackend::Impl::outstream_destroy(IoConnection* conn)
{
    Impl* io = (Impl*)(conn->_owner);
    IoBackendConnection* backend_conn = (IoBackendConnection*)conn;
    io->_online_register.unset(conn->_id);
    io->_cache.recycle(backend_conn);
}

int IoBackend::Impl::createSocket(uint64_t key)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        LOG(ERROR) << "create socket error\n";
        return -1;
    }

    int flags = 1;
    int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
    if (ret != 0)
        LOG(ERROR) << "setsockopt tcp_nodelay error\n";

    struct linger ling = {0, 0};
    ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
    if (ret != 0)
        LOG(ERROR) << "setsockopt tcp_linger error\n";

    if (util::setNonblocking(fd) == false)
    {
        LOG(ERROR) << "set socket nonblocking error\n";
        ::close(fd);
        return -1;
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(key & 0x0FFFFFFFF);
    sin.sin_addr.s_addr = key >> 32;

    connect(fd, (struct sockaddr*)&sin, sizeof(sin));

    return fd;
}

void IoBackend::Impl::createEvent()
{
    int notify[2];
    //int ret = pipe(notify);
    int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, notify);
    if (ret == -1)
    {
        LOG(ERROR) << "socketpair error, abort\n";
        abort();
    }

    _new_rnotify = notify[0];
    _new_wnotify = notify[1];

    if (util::setNonblocking(_new_rnotify) == false)
    {
        LOG(ERROR) << "setNonblocking error, abort\n";
        abort();
    }

    //ret = pipe(notify);
    ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, notify);
    if (ret == -1)
    {
        LOG(ERROR) << "socketpair error, abort\n";
        abort();
    }

    _push_rnotify = notify[0];
    _push_wnotify = notify[1];

    if (util::setNonblocking(_push_rnotify) == false)
    {
        LOG(ERROR) << "setNonblocking error, abort\n";
        abort();
    }
}

int IoBackend::Impl::run()
{
    LOG(INFO) << "backend io(" << _id << ") running ...\n";

    createEvent();

    event_set(&_new_event, _new_rnotify, EV_READ|EV_PERSIST, connectionHandle, this);
    event_base_set(_event_base, &_new_event); 
    event_add(&_new_event, 0); 

    event_set(&_push_event, _push_rnotify, EV_READ|EV_PERSIST, pushHandle, this);
    event_base_set(_event_base, &_push_event); 
    event_add(&_push_event, 0); 

    event_base_loop(_event_base, 0);

    LOG(INFO) << "backend io(" << _id << ") quit ...\n";

    return 0;
}

int IoBackend::Impl::stop()
{
    return -1;
}

void IoBackend::Impl::newConnection(int fd)
{
    int count = 0;
    BackendSocketQueue::QueueElement* list = _sock_queue->get();
    BackendSocketQueue::QueueElement* item;
    while (list)
    {
        ++count;

        int new_fd = list->_value;
        uint64_t conn_id = list->_id;
        item = list;
        list = item->_next;
        delete item;

        IoBackendConnection* conn = _cache.get();
        conn->initialize(new_fd,
                _id,
                conn_id,
                this,
                _event_base,
                _workers);
        conn->setReadTimeout(_rtimeout);
        conn->setWriteTimeout(_wtimeout);
        conn->setWhenDestroy(destroy);
        conn->setRead(true);
    }

    ++_cache._loop_count;

    if (count > 256)
        count = 256;
    else if (count == 0)
        count = 1;

    char buf[256];
    int ret = ::read(fd, buf, count);
    (void)ret;
}

void IoBackend::Impl::connectionHandle(int fd, short which, void* v)
{
    (void)which;
    IoBackend::Impl* s = (IoBackend::Impl*)v;
    s->newConnection(fd);
}

void IoBackend::Impl::pushMessage(ms::IMessage* msg)
{
    IoBackendConnection* conn = _online_register.get(msg->_conn);
    if (conn)
    {
        conn->setWrite(true);
        conn->pushMsgData(msg);
    }
    else
    {
        int fd = createSocket(msg->_conn);
        if (fd != -1)
        {
            IoBackendConnection* conn = new (std::nothrow) IoBackendConnection();
            if (conn == 0)
            {
                LOG(ERROR) << "memory error, abort\n";
                abort();
            }
            conn->initialize(fd,
                    _id,
                    msg->_conn,
                    this,
                    _event_base,
                    0);
            _online_register.set(msg->_conn, conn);
            conn->setWhenDestroy(outstream_destroy);
            conn->setConnecting();
            conn->pushMsgData(msg);
        }
        else
        {
            releaseIMessage(msg);
        }
    }
}

void IoBackend::Impl::push(int fd)
{
    int count = 0;
    ms::IMessage* list = _msg_queue->get();
    ms::IMessage* item;
    while (list)
    {
        ++count;

        item = list;
        list = list->_next;
        item->_next = 0;

        pushMessage(item);
    }

    if (count > 256)
        count = 256;
    else if (count == 0)
        count = 1;

    char buf[256];
    int ret = ::read(fd, buf, count);
    (void)ret;
}

void IoBackend::Impl::pushHandle(int fd, short which, void* v)
{
    (void)which;
    IoBackend::Impl* s = (IoBackend::Impl*)v;
    s->push(fd);
}

/*******************************************************************************
************************************IoBackend*********************************
*******************************************************************************/
IoBackend::IoBackend(int id,
        BackendSocketQueue* sock_queue,
        BackendMessageQueue* msg_queue,
        ms::MsWorkerCluster* workers):
    _core(new (std::nothrow) IoBackend::Impl(this,
                id,
                sock_queue,
                msg_queue,
                workers))
{
    if (_core == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }
}

IoBackend::~IoBackend()
{
    delete _core;
}

int IoBackend::run()
{
    util::PosixThreadFactory ptf;
    ptf.setPolicy(util::PosixThreadFactory::OTHER);
    ptf.setStackSize(10);
    ptf.setDetached(true);

    boost::shared_ptr<util::Runnable> core(_core);
    boost::shared_ptr<util::Thread> thread = ptf.newThread(core);
    thread->run();

    return 0;
}

int IoBackend::stop()
{
    return _core->stop();
}

void IoBackend::newNotify()
{
    _core->newNotify();
}

void IoBackend::pushNotify()
{
    _core->pushNotify();
}

int IoBackend::id() const
{
    return _core->id();
}

void IoBackend::setReadTimeout(int timeout)
{
    _core->setReadTimeout(timeout);
}

void IoBackend::setWriteTimeout(int timeout)
{
    _core->setWriteTimeout(timeout);
}

}
