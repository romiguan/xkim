#ifndef __IO_APP_CLUSTER_H__
#define __IO_APP_CLUSTER_H__

#include "io_app.h"
#include "ms/ms_message.h"

#include <assert.h>

namespace io
{

class IoAppCluster
{
    public:
        explicit IoAppCluster(IoProcessor* processor);
        ~IoAppCluster();

        int initialize(const char* config);
        int run();

        int dispatchConnection(int fd, uint64_t id);
        int dispatchPushList(int io, ms::IMessage* msg);

        int size() const { return _cluster_size; }
        void setSocketMax(int max) { _socket_max = max; }

        void setPendingMessageCollector(ms::MsMessageCollector* pending_message_collector)
        {
            _pending_message_collector = pending_message_collector;
        }

        void setClusterSize(int size)
        {
            _cluster_size = size;
        }

        void setReadTimeout(int timeout)
        {
            _read_timeout = timeout;
        }

        void setWriteTimeout(int timeout)
        {
            _write_timeout = timeout;
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

    private:
        int _cluster_index;
        int _cluster_size;
        int _socket_max;
        int _read_timeout;     // seconds
        int _write_timeout;    // seconds
        int _second_limit;
        int _minute_limit;
        int _hour_limit;

        SocketQueue* _sock_queue;
        MessageQueue* _msg_queue;
        IoProcessor* _processor;
        ms::MsMessageCollector* _pending_message_collector;
        IoApp** _core_io;
};

}

#endif
