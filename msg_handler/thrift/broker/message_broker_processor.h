#ifndef __MESSAGE_BROKER_PROCESSOR_H__
#define __MESSAGE_BROKER_PROCESSOR_H__

#include "framework/ms/ms_message_processor.h"
#include "framework/ms/ms_message_collector.h"
#include "framework/ms/ms_message.h"
#include "message/im_user_message_router.h"
#include "message/im_message_parser.h"

namespace im
{

class MessageBrokerProcessor : public ms::MsMessageProcessor
{
    public:
        MessageBrokerProcessor();
        virtual ~MessageBrokerProcessor();

        void setMessageRouter(im::ImUserMessageRouter* router)
        {
            _msg_router = router;
        }

        void setPendingMessageCollector(ms::MsMessageCollector* collector)
        {
            _pending_msg_collector = collector;
        }

        void setMessageParser(im::ImMessageParser* parser)
        {
            _parser = parser;
        }

        virtual int process(ms::IMessage* msg);

    private:
        int processMsgPack(IMessagePack* pack);
        void processErrorPack(ms::IMessage* msg);

    private:
        im::ImUserMessageRouter* _msg_router;
        ms::MsMessageCollector* _pending_msg_collector;
        im::ImMessageParser* _parser;
};

}

#endif
