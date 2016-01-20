#include "io_app.h"
#include "io_app_connection.h"
#include "connection_cache.h"
#include "connection_register.h"
#include "ms/ms_message.h"
#include "lib/util.h"
#include "lib/thread.h"
#include "lib/posix_thread_factory.h"

#include <event.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <glog/logging.h>

namespace io
{

class IoApp::Impl : public util::Runnable
{
    public:
        Impl(IoApp* owner,
                int id,
                SocketQueue* sock_queue,
                MessageQueue* msg_queue,
                IoProcessor* processor);

        virtual ~Impl();

        virtual int run();
        virtual int stop();

        void setReadTimeout(int timeout) { _rtimeout = timeout; }
        void setWriteTimeout(int timeout) { _wtimeout = timeout; }

        void setPendingMessageCollector(ms::MsMessageCollector* pending_message_collector)
        {
            _pending_message_collector = pending_message_collector;
        }

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

        void setSecondLimit(int limit)
        {
            _second_limit = limit;
            assert(_second_limit > 0);
        }

        void setMinuteLimit(int limit)
        {
            _minute_limit = limit;
            assert(_minute_limit > 0);
        }

        void setHourLimit(int limit)
        {
            _hour_limit = limit;
            assert(_hour_limit > 0);
        }

        int id() const { return _id; }
        int connCount() const { return _conn_count; }

    private:
        static void connectionHandle(int fd, short which, void* v);
        static void pushHandle(int fd, short which, void* v);

        static void destroy(IoConnection* conn);

    private:
        void createEvent();
        void newConnection(int fd);
        void pushMessage(ms::IMessage* msg);
        void push(int fd);

    private:
        IoApp* _owner;
        int _id;

        int _rtimeout;
        int _wtimeout;

        int _new_rnotify;
        int _new_wnotify;
        int _push_rnotify;
        int _push_wnotify;

        int _second_limit;
        int _minute_limit;
        int _hour_limit;

        volatile int _conn_count;
        struct event_base* _event_base;
        struct event _new_event;
        struct event _push_event;
        SocketQueue* _sock_queue;
        MessageQueue* _msg_queue;
        IoProcessor* _processor;
        ms::MsMessageCollector* _pending_message_collector;
        ConnectionCache<IoAppConnection> _cache;
        ConnectionRegister<IoAppConnection> _online_register;
};

IoApp::Impl::Impl(IoApp* owner,
        int id,
        SocketQueue* sock_queue,
        MessageQueue* msg_queue,
        IoProcessor* processor):
    _owner(owner),
    _id(id),
    _rtimeout(0),
    _wtimeout(0),
    _second_limit(0x0FFFFFFF),
    _minute_limit(0x0FFFFFFF),
    _hour_limit(0x0FFFFFFF),
    _conn_count(0),
    _event_base(event_base_new()),
    _sock_queue(sock_queue),
    _msg_queue(msg_queue),
    _processor(processor),
    _pending_message_collector(0),
    _cache(id)
{
    if (_event_base == 0)
    {
        LOG(ERROR) << "IoApp construct error, abort\n";
        abort();
    }
}

IoApp::Impl::~Impl()
{
    if (_event_base)
        event_base_free(_event_base);
}

void IoApp::Impl::destroy(IoConnection* conn)
{
    Impl* io = (Impl*)(conn->_owner);
    IoAppConnection* app_conn = (IoAppConnection*)conn;

    --io->_conn_count;
    io->_online_register.unset(conn->_id);
    io->_cache.recycle(app_conn);
}

void IoApp::Impl::createEvent()
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

int IoApp::Impl::run()
{
    LOG(INFO) << "im io(" << _id << ") running ...\n";

    createEvent();

    event_set(&_new_event, _new_rnotify, EV_READ|EV_PERSIST, connectionHandle, this);
    event_base_set(_event_base, &_new_event); 
    event_add(&_new_event, 0); 

    event_set(&_push_event, _push_rnotify, EV_READ|EV_PERSIST, pushHandle, this);
    event_base_set(_event_base, &_push_event); 
    event_add(&_push_event, 0); 

    event_base_loop(_event_base, 0);

    LOG(INFO) << "im io(" << _id << ") quit ...\n";

    return 0;
}

int IoApp::Impl::stop()
{
    return -1;
}

void IoApp::Impl::newConnection(int fd)
{
    int count = 0;
    SocketQueue::QueueElement* list = _sock_queue->get();
    SocketQueue::QueueElement* item;
    while (list)
    {
        ++count;
        ++_conn_count;

        int new_fd = list->_value;
        uint64_t conn_id = list->_id;
        item = list;
        list = item->_next;
        SocketQueue::freeQueueElement(item);

        IoAppConnection* conn = _cache.get();
        conn->initialize(new_fd,
                _id,
                conn_id,
                this,
                _event_base,
                _processor);

        conn->setReadTimeout(_rtimeout);
        conn->setWriteTimeout(_wtimeout);
        conn->setWhenDestroy(destroy);
        conn->setSecondLimit(_second_limit);
        conn->setMinuteLimit(_minute_limit);
        conn->setHourLimit(_hour_limit);
        conn->setPendingMessageCollector(_pending_message_collector);
        conn->setRead(true);

        _online_register.set(conn_id, conn);
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

void IoApp::Impl::connectionHandle(int fd, short which, void* v)
{
    (void)which;
    IoApp::Impl* s = (IoApp::Impl*)v;
    s->newConnection(fd);
}

void IoApp::Impl::pushMessage(ms::IMessage* msg)
{
    IoAppConnection* conn = _online_register.get(msg->_conn);
    if (conn)
    {
        if (msg->_switch_state)
            msg->_switch_state(conn, msg->_state);

        if (msg->_push_callback)
            msg->_push_callback(conn);

        conn->setWrite(true);
        conn->pushMsgData(msg);
        //fprintf(stderr, "push list: %d\n", conn->pushListSize());
    }
    else
    {
        if (_pending_message_collector)
        {
            LOG(INFO) << "connection was closed, collect the message\n";
            _pending_message_collector->collect(msg);
        }
        else
        {
            LOG(INFO) << "connection was closed, drop the message\n";
            releaseIMessage(msg);
        }
    }
}

void IoApp::Impl::push(int fd)
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

void IoApp::Impl::pushHandle(int fd, short which, void* v)
{
    (void)which;
    IoApp::Impl* s = (IoApp::Impl*)v;
    s->push(fd);
}

/*******************************************************************************
************************************IoApp****************************************
*******************************************************************************/
IoApp::IoApp(int id,
        SocketQueue* sock_queue,
        MessageQueue* msg_queue,
        IoProcessor* processor):
    _core(new (std::nothrow) IoApp::Impl(this,
                id,
                sock_queue,
                msg_queue,
                processor))
{
    if (_core == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }
}

IoApp::~IoApp()
{
    delete _core;
}

int IoApp::run()
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

int IoApp::stop()
{
    return _core->stop();
}

void IoApp::newNotify()
{
    _core->newNotify();
}

void IoApp::pushNotify()
{
    _core->pushNotify();
}

int IoApp::id() const
{
    return _core->id();
}

void IoApp::setReadTimeout(int timeout)
{
    _core->setReadTimeout(timeout);
}

void IoApp::setWriteTimeout(int timeout)
{
    _core->setWriteTimeout(timeout);
}

void IoApp::setPendingMessageCollector(ms::MsMessageCollector* pending_message_collector)
{
    _core->setPendingMessageCollector(pending_message_collector);
}

int IoApp::connCount() const
{
    return _core->connCount();
}

void IoApp::setSecondLimit(int limit)
{
    _core->setSecondLimit(limit);
}

void IoApp::setMinuteLimit(int limit)
{
    _core->setMinuteLimit(limit);
}

void IoApp::setHourLimit(int limit)
{
    _core->setHourLimit(limit);
}

}
