#ifndef __IM_MESSAGE_AUTH_PROCESSOR_H__
#define __IM_MESSAGE_AUTH_PROCESSOR_H__

#include "framework/ms/ms_message_processor.h"
#include "framework/ms/ms_message.h"

namespace im
{

struct IMessagePack;
class ImUserMessageRouter;
class RouteInfoPublisher;
class ImMessageParser;
class ImMessageAuthProcessor : public ms::MsMessageProcessor
{
    public:
        explicit ImMessageAuthProcessor(ImUserMessageRouter* router);
        virtual ~ImMessageAuthProcessor();

        virtual int process(ms::IMessage* msg);
        void initialize(const char* config);

    private:
        int processErrorPack(ms::IMessage* msg);
        int processAuthPack(IMessagePack* pack);
        int auth(IMessagePack* pack);

    private:
        ImUserMessageRouter* _msg_router;
        ImMessageParser* _parser;
        RouteInfoPublisher* _route_publisher;
};

}

#endif
