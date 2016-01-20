#ifndef __IO_APP_CONNECTION_H__
#define __IO_APP_CONNECTION_H__

#include "io_connection.h"
#include "io_sampling.h"
#include "io_push_list.h"
#include "connection_cache.h"
#include "connection_register.h"
#include "ms/ms_message.h"
#include "ms/ms_message_collector.h"

#include <stdlib.h>
#include <event.h>

namespace io
{

class IoProcessor;
class IoAppConnection : public IoConnection
{
    public:
        IoAppConnection();
        virtual ~IoAppConnection();

        void initialize(int fd,
                int io,
                uint64_t  id,
                void* owner,
                struct event_base* event_base,
                IoProcessor* processor);

        void setSecondLimit(int limit) { _second_limit = limit; }
        void setMinuteLimit(int limit) { _minute_limit = limit; }
        void setHourLimit(int limit) { _hour_limit = limit; }
        void setState(int state) { _state = state; }

        void setPendingMessageCollector(ms::MsMessageCollector* pending_message_collector)
        {
            _pending_message_collector = pending_message_collector;
        }

        void pushMsgData(ms::IMessage* msg);

    protected:
        virtual int whenReceivedFrame(char* buf, int size);
        virtual int whenClosed();
        virtual IoPushData* getPushData();
        virtual void pendingPushData(IoPushData* data);

    private:
        int _state; // 链接状态机, 状态0保留, 表示第一个pack
        IoProcessor* _processor;
        ms::MsMessageCollector* _pending_message_collector;
        IoSampling _sampling;
        IoPushBuffer _push_buffer;
        IoPushData* _pending_data;

        int _second_limit;
        int _minute_limit;
        int _hour_limit;
};

}

#endif
