#ifndef __MESSAGE_BROKER_H__
#define __MESSAGE_BROKER_H__

#include "message_proxy.h"
#include "framework/ms/ms_message_collector.h"
#include "framework/ms/ms_message.h"
#include "framework/lib/thread.h"
#include "message/im_user_message_router.h"
#include "thrift/pilot/pooled_client.h"
#include "thrift/gen/Message.h"

using namespace ms;
using namespace Pilot;

namespace im
{

class MessageBroker
{
    public:
        MessageBroker():
            _msg_router(0),
            _pending_msg_collector(0)
        {
        }
        ~MessageBroker() {}

        void setMessageRouter(im::ImUserMessageRouter* router)
        {
            _msg_router = router;
        }

        void setPendingMessageCollector(ms::MsMessageCollector* collector)
        {
            _pending_msg_collector = collector;
        }

        int initialize(const char* conf);
        int dispatch(ms::IMessage* msg);

        void run();
        void join();

    private:
        im::ImUserMessageRouter* _msg_router;
        ms::MsMessageCollector* _pending_msg_collector;
        boost::shared_ptr<util::Thread> _thread;
        PooledClient(Message)* _service;
        char _conf[PATH_MAX];
        int _port;
        int _worker_num;
        int _io_num;
};

}

#endif
