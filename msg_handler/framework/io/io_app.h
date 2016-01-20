#ifndef __IO_APP_H__
#define __IO_APP_H__

#include "lib/queue.h"
#include "ms/ms_message_queue.h"
#include "ms/ms_message_collector.h"

namespace io
{

typedef util::Queue SocketQueue;
typedef ms::MsMessageQueue MessageQueue;

class IoProcessor;
class IoApp
{
    public:
        IoApp(int id,
                SocketQueue* sock_queue,
                MessageQueue* msg_queue,
                IoProcessor* processor);
        virtual ~IoApp();

        int run();
        int stop();

        void newNotify();
        void pushNotify();
        void setReadTimeout(int timeout);
        void setWriteTimeout(int timeout);
        void setPendingMessageCollector(ms::MsMessageCollector* pending_message_collector);

        void setSecondLimit(int limit);
        void setMinuteLimit(int limit);
        void setHourLimit(int limit);

        int id() const;
        int connCount() const;

    private:
        class Impl;
        Impl* _core;
};

}

#endif
