#include "im_message_dispatcher.h"
#include "im_app_worker_cluster.h"
#include "im_message_define.h"
#include "im_message_processor.h"
#include "im_message_auth_processor.h"
#include "im_auth_worker_cluster.h"
#include "route_info_publisher.h"
#include "framework/lib/ini_file_reader.h"

#include <stdio.h>

namespace im
{

ImMessageDispatcher::ImMessageDispatcher(ImUserMessageRouter* router, MessageBroker* broker):
    _app_workers(0),
    _auth_workers(0),
    _msg_router(router),
    _broker(broker),
    _pending_msg_collector(0)
{
}

ImMessageDispatcher::~ImMessageDispatcher()
{
}

int ImMessageDispatcher::initialize(const char* config)
{
    assert(_msg_router);
    assert(_broker);

    util::IniFileReader ini_reader(config);

    ImMessageAuthProcessor* auth_processor = new ImMessageAuthProcessor(_msg_router);
    auth_processor->initialize(config);
    int auth_thread_num = ini_reader.IniGetIntValue("APP", "auth_thread_num", 1);
    if (auth_thread_num <= 0)
        auth_thread_num = 1;
    _auth_workers = new ImAuthWorkerCluster(auth_thread_num, auth_processor);
    _auth_workers->run();

    ImMessageProcessor* processor = new ImMessageProcessor(_msg_router, _broker);
    processor->setPendingMessageCollector(_pending_msg_collector);
    int app_thread_num = ini_reader.IniGetIntValue("APP", "app_thread_num", 1);
    if (app_thread_num <= 0)
        app_thread_num = 1;
    _app_workers = new ImAppWorkerCluster(app_thread_num, processor);
    _app_workers->run();

    return 0;
}

int ImMessageDispatcher::process(ms::IMessage* msg)
{
    switch (msg->_state)
    {
        case INIT_STATE:
            msg->_state = AUTH_STATE;
        case AUTH_STATE:
            msg->_priority = 0;
            _auth_workers->schedule(msg);
            break;
        case MSG_STATE:
        default:
            msg->_priority = 1;
            _app_workers->schedule(msg);
            break;
    }
    return 0;
}

}
