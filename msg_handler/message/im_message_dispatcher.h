#ifndef __IM_MESSAGE_DISPATCHER_H__
#define __IM_MESSAGE_DISPATCHER_H__

#include "framework/io/io_processor.h"
#include "framework/ms/ms_message_collector.h"

namespace im
{

class ImUserMessageRouter;
class ImAppWorkerCluster;
class ImAuthWorkerCluster;
class MessageBroker;
class ImMessageDispatcher : public io::IoProcessor
{
    public:
        ImMessageDispatcher(ImUserMessageRouter* router, MessageBroker* broker);
        virtual ~ImMessageDispatcher();

        virtual int process(ms::IMessage* msg);

        int initialize(const char* config);

        void setPendingMessageCollector(ms::MsMessageCollector* collector)
        {
            _pending_msg_collector = collector;
        }

    private:
        ImAppWorkerCluster* _app_workers;
        ImAuthWorkerCluster* _auth_workers;
        ImUserMessageRouter* _msg_router;
        MessageBroker* _broker;
        ms::MsMessageCollector* _pending_msg_collector;
};

}

#endif
