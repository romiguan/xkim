#ifndef __IM_MESSAGE_PROCESSOR_H__
#define __IM_MESSAGE_PROCESSOR_H__

#include "framework/ms/ms_message_processor.h"
#include "framework/ms/ms_message_collector.h"
#include "framework/ms/ms_message.h"
#include "framework/lib/util.h"
#include "thrift/broker/message_broker.h"

namespace im
{

struct IMessagePack;
class ImUserMessageRouter;
class ImMessageParser;
class RouteInfoPublisher;
class ImMessageProcessor : public ms::MsMessageProcessor
{
    public:
        ImMessageProcessor(ImUserMessageRouter* router, MessageBroker* broker);
        virtual ~ImMessageProcessor();

        virtual int process(ms::IMessage* msg);

        void setPendingMessageCollector(ms::MsMessageCollector* collector)
        {
            _pending_msg_collector = collector;
        }

        void setRouteInfoPublisher(im::RouteInfoPublisher* ripub)
        {
            _route_publisher = ripub;
        }

    private:
        int genMsgId(util::MessageId& msg_id);
        int processErrorPack(ms::IMessage* msg);
        int processTouchPack(IMessagePack* pack);
        int processMsgPack(IMessagePack* pack);
        int processAuthPack(IMessagePack* pack);

    private:
        unsigned int _host_id;
        ImUserMessageRouter* _msg_router;
        MessageBroker* _broker;
        ImMessageParser* _parser;
        ms::MsMessageCollector* _pending_msg_collector;
        RouteInfoPublisher* _route_publisher;
};

}

#endif
