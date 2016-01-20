#include "message_broker_processor.h"

#include <assert.h>
#include <stdio.h>

#include <glog/logging.h>

namespace im
{

MessageBrokerProcessor::MessageBrokerProcessor():
    _msg_router(0),
    _pending_msg_collector(0),
    _parser(0)
{
}

MessageBrokerProcessor::~MessageBrokerProcessor()
{
}

int MessageBrokerProcessor::process(ms::IMessage* msg)
{
    assert(_msg_router);
    assert(_parser);

    IMessagePack* pack = _parser->parse(msg);
    if (pack == 0)
    {
        processErrorPack(msg);
        return -1;
    }

    int ret = -1;
    IMessagePackType pack_type = _parser->getMessagePackType(pack);
    switch (pack_type)
    {
        case MSG:
            ret = processMsgPack(pack);
            break;
        default:
            ret = -1;
            break;
    }

    releaseIMessagePack(pack);

    if (ret == -1)
        processErrorPack(msg);

    return ret;
}

int MessageBrokerProcessor::processMsgPack(IMessagePack* pack)
{
    UID uid = _parser->getMsgPackUser(pack);
    if (uid == -1)
        return -1;

    ms::IMessage* msg = _parser->createLocalMsgPack(pack);
    if (msg)
    {
        ms::IMessage* pending = _msg_router->routeList(msg);
        if (pending)
        {
            if (_pending_msg_collector)
            {
                LOG(INFO) << "route local failed, collect the message\n";
                _pending_msg_collector->collectList(pending);
            }
            else
            {
                LOG(INFO) << "route local failed, drop the message\n";
                ms::IMessage* trans;
                while (pending)
                {
                    trans = pending;
                    pending = pending->_next;
                    ms::releaseIMessage(trans);
                }
            }
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

void MessageBrokerProcessor::processErrorPack(ms::IMessage* msg)
{
    ms::releaseIMessage(msg);
}

}
