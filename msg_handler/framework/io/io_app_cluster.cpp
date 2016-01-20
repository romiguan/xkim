#include "io_app_cluster.h"
#include "lib/queue.h"
#include "lib/ini_file_reader.h"

#include <stdio.h>
#include <stdlib.h>

#include <glog/logging.h>

namespace io
{

IoAppCluster::IoAppCluster(IoProcessor* processor):
    _cluster_index(0),
    _cluster_size(0),
    _socket_max(0x0FFFFFFF),
    _read_timeout(0),
    _write_timeout(0),
    _second_limit(0x0FFFFFFF),
    _minute_limit(0x0FFFFFFF),
    _hour_limit(0x0FFFFFFF),

    _sock_queue(0),
    _msg_queue(0),
    _processor(processor),
    _pending_message_collector(0),
    _core_io(0)
{
}

IoAppCluster::~IoAppCluster()
{
    for (int i = 0; i < _cluster_size; ++i)
        delete _core_io[i];

    delete [] _core_io;
    delete [] _sock_queue;
    delete [] _msg_queue;
}

int IoAppCluster::initialize(const char* config)
{
    util::IniFileReader ini_reader(config);

    int io_thread_num = ini_reader.IniGetIntValue("IO", "io_thread_num", 1);
    if (io_thread_num <= 0)
        io_thread_num = 1;
    int io_max = ini_reader.IniGetIntValue("IO", "io_max", 1000);
    if (io_max <= 0)
        io_max = 1000;
    int io_read_timeout = ini_reader.IniGetIntValue("IO", "io_read_timeout", 10);
    if (io_read_timeout <= 0)
        io_read_timeout = 10;
    int io_write_timeout = ini_reader.IniGetIntValue("IO", "io_write_timeout", 10);
    if (io_write_timeout <= 0)
        io_write_timeout = 10;
    int io_request_limit_second = ini_reader.IniGetIntValue("IO", "io_request_limit_second", 3);
    if (io_request_limit_second <= 0)
        io_request_limit_second = 3;
    int io_request_limit_minute = ini_reader.IniGetIntValue("IO", "io_request_limit_minute", 30);
    if (io_request_limit_minute <= 0)
        io_request_limit_minute = 30;
    int io_request_limit_hour = ini_reader.IniGetIntValue("IO", "io_request_limit_hour", 500);
    if (io_request_limit_hour <= 0)
        io_request_limit_hour = 500;

    setClusterSize(io_thread_num);
    setSocketMax(io_max);
    setReadTimeout(io_read_timeout);
    setWriteTimeout(io_write_timeout);
    setSecondLimit(io_request_limit_second);
    setMinuteLimit(io_request_limit_minute);
    setHourLimit(io_request_limit_hour);

    return 0;
}

int IoAppCluster::run()
{
    if (_cluster_size <= 0)
        return -1;

    _sock_queue = new (std::nothrow) SocketQueue[_cluster_size];
    if (_sock_queue == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    _msg_queue = new (std::nothrow) MessageQueue[_cluster_size];
    if (_msg_queue == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    _core_io = new (std::nothrow) IoApp*[_cluster_size];
    if (_core_io == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    for (int i = 0; i < _cluster_size; ++i)
    {
        IoApp* io = new (std::nothrow) IoApp(i, &_sock_queue[i], &_msg_queue[i], _processor);
        if (io == 0)
        {
            LOG(ERROR) << "memory error, abort\n";
            abort();
        }

        io->setReadTimeout(_read_timeout);
        io->setWriteTimeout(_write_timeout);
        io->setPendingMessageCollector(_pending_message_collector);
        io->setSecondLimit(_second_limit);
        io->setMinuteLimit(_minute_limit);
        io->setHourLimit(_hour_limit);
        _core_io[i] = io;

        io->run();
    }

    return 0;
}

int IoAppCluster::dispatchConnection(int fd, uint64_t id)
{
    int socket_count = 0;
    int min = _socket_max;
    int to = 0;
    for (int i = 0; i < _cluster_size; ++i)
    {
        IoApp* io = _core_io[i];
        int c = io->connCount();
        socket_count += c;
        if (c < min)
        {
            min = c;
            to = i;
        }
    }

    if (socket_count < _socket_max)
    {
        LOG(INFO) << "new connection to im(" << to << "), socket: " << fd << ", conn: " << id << std::endl;
        //_cluster_index = (_cluster_index + 1) % _cluster_size;
        _sock_queue[to].put(fd, id);
        _core_io[to]->newNotify();
        return 0;
    }
    else
    {
        LOG(INFO) << "connection reach the max, drop\n";
        ::close(fd);
        return -1;
    }
}

int IoAppCluster::dispatchPushList(int io, ms::IMessage* msg)
{
    _msg_queue[io].put(msg);
    _core_io[io]->pushNotify();
    return 0;
}

}
