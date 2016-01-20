#include "im_message_auth_processor.h"
#include "im_user_message_router.h"
#include "im_message_parser.h"
#include "im_message_define.h"
#include "route_info_publisher.h"
#include "framework/lib/ini_file_reader.h"

#include <glog/logging.h>

extern uint64_t g_broker_instance;

namespace im
{

ImMessageAuthProcessor::ImMessageAuthProcessor(ImUserMessageRouter* router):
    _msg_router(router),
    _parser(new ImMessageParser()),
    _route_publisher(0)
{
    assert(_msg_router);
    assert(_parser);
}

ImMessageAuthProcessor::~ImMessageAuthProcessor()
{
    delete _parser;
}

void ImMessageAuthProcessor::initialize(const char* config)
{
    util::IniFileReader ini_reader(config);

    std::string redis_host = ini_reader.IniGetStrValue("PUB_REDIS", "redis_host", "");
    assert(redis_host.empty() == false);

    int redis_port = ini_reader.IniGetIntValue("PUB_REDIS", "redis_port", 0);
    assert(redis_port > 0);

    int redis_db_index = ini_reader.IniGetIntValue("PUB_REDIS", "redis_db", -1);
    assert(redis_db_index >= 0);

    std::string redis_channel = ini_reader.IniGetStrValue("PUB_REDIS", "redis_channel", "");
    assert(redis_channel.empty() == false);

    int redis_io_timeout = ini_reader.IniGetIntValue("PUB_REDIS", "redis_io_timeout", -1);
    assert(redis_io_timeout > 0);

    const struct timeval conn_to = {0, 20000};
    const struct timeval io_to = {0, redis_io_timeout*1000};

    // init RouteInfoPublisher, refered by ImMessageProcessorSimple
    _route_publisher = new im::RouteInfoPublisher();
    _route_publisher->init(redis_host, redis_port, conn_to, io_to, redis_channel, redis_db_index);
}

int ImMessageAuthProcessor::processErrorPack(ms::IMessage* msg)
{
    msg->_need_collect = 0;
    msg->_force_push = 1;
    msg->_close_conn = 1;
    gettimeofday(&msg->_process_end_time, 0);
    _msg_router->routeBack(msg);
    return 0;
}

int ImMessageAuthProcessor::auth(IMessagePack* pack)
{
    UID uid = _parser->getAuthPackUid(pack);
    if (uid == -1)
        return -1;

    const std::string& token = _parser->getAuthPackToken(pack);
    if (token.empty())
        return -1;

    //TODO 认证过程

    return 0;
}

int ImMessageAuthProcessor::processAuthPack(IMessagePack* pack)
{
    if (_parser->checkTimeStamp(pack) == -1)
        return -1;

    if (pack->_msg->_state != AUTH_STATE)
        return -1;

    if (auth(pack) == 0)
    {
        UID uid = _parser->getAuthPackUid(pack);
        ms::IMessage* msg = _parser->createAuthPack(uid, pack);
        msg->_state = MSG_STATE;
        msg->_switch_state = switch_state_app_msg;

        _msg_router->update(msg);
        _msg_router->routeBack(msg);

        // set UID->MSG_HANDLER mapping to redis
        // and then PUBLISH it to redis channel.
        // Very important information, must be called in synchorize mode in this thread
        assert(_route_publisher);
        if (!_route_publisher->publishRouteInfo(uid, g_broker_instance)) 
        {
            // TODO: currently, only output ERROR LOG, will add more process later
            // And if PUBLISH failed, not handle it as a fail case
            LOG(ERROR) << "PUBLISH routing info failed.";
        }

        return 0;
    }

    return -1;
}

int ImMessageAuthProcessor::process(ms::IMessage* msg)
{
    gettimeofday(&msg->_process_begin_time, 0);

    IMessagePack* pack = _parser->parse(msg);
    if (pack == 0)
    {
        LOG(INFO) << "illeagle protobuf message, kick it...\n";
        processErrorPack(msg);
        return -1;
    }

    int ret = -1;
    IMessagePackType pack_type = _parser->getMessagePackType(pack);
    switch (pack_type)
    {
        case AUTH:
            ret = processAuthPack(pack);
            break;
        default:
            ret = -1;
            break;
    }

    releaseIMessagePack(pack);

    if (ret == 0)
    {
        return 0;
    }
    else
    {
        LOG(INFO) << "illeagle auth message, kick it...\n";
        processErrorPack(msg);
        return -1;
    }
}
}

