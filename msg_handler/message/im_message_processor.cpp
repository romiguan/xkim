#include "im_message_processor.h"
#include "im_user_message_router.h"
#include "im_message_parser.h"
#include "im_message_define.h"
#include "framework/lib/util.h"
#include "route_info_publisher.h"

#include <assert.h>

#include <glog/logging.h>

namespace im
{

ImMessageProcessor::ImMessageProcessor(ImUserMessageRouter* router, MessageBroker* broker):
    _msg_router(router),
    _broker(broker),
    _parser(new ImMessageParser()),
    _pending_msg_collector(0),
    _route_publisher(0)
{
    assert(_msg_router);
    assert(_broker);
    assert(_parser);

    char host_name[256];
    gethostname(host_name, 256);
    _host_id = util::bkdrHash(host_name, strlen(host_name));
}

ImMessageProcessor::~ImMessageProcessor()
{
    delete _parser;
}

int ImMessageProcessor::genMsgId(util::MessageId& msg_id)
{
    static __thread unsigned int seed = 0;
    static __thread util::Counter* counter = 0;

    if (expect_false(seed == 0))
        seed = util::gettid();

    if (expect_false(counter == 0))
        counter = new (std::nothrow) util::Counter();

    util::MessageIdDetail& detail = msg_id._id;
    detail._time = time(0);
    detail._rand1 = rand_r(&seed) % 4095;
    detail._tid = util::gettid();
    detail._host_id = _host_id;
    detail._counter = counter->get();
    detail._rand2 = rand_r(&seed) % 65535;

    msg_id._long[0] = util::murmurHash64B(msg_id._bytes, sizeof(util::MessageId), 0x0EE6B27EB);

    return 8;
}

int ImMessageProcessor::processErrorPack(ms::IMessage* msg)
{
    msg->_need_collect = 0;
    msg->_force_push = 1;
    msg->_close_conn = 1;
    gettimeofday(&msg->_process_end_time, 0);
    _msg_router->routeBack(msg);
    return 0;
}

int ImMessageProcessor::processTouchPack(IMessagePack* pack)
{
    if (_parser->checkTimeStamp(pack) == -1)
        return -1;

    if (pack->_msg->_state != MSG_STATE)
        return -1;

    UID uid = _parser->getTouchPackUid(pack);
    if (uid == -1)
        return -1;

    ms::IMessage* msg = _parser->createTouchAckPack(uid, pack);
    _msg_router->update(msg);
    _msg_router->routeBack(msg);

    return 0;
}

int ImMessageProcessor::processMsgPack(IMessagePack* pack)
{
    if (_parser->checkTimeStamp(pack) == -1)
        return -1;

    if (pack->_msg->_state != MSG_STATE)
        return -1;

    UID uid = _parser->getMsgPackUser(pack);
    if (uid == -1)
        return -1;

    int target_uid_count = _parser->getMsgPackTargetUserCount(pack);
    if (target_uid_count <= 0)
        return -1;

    util::MessageId mid;
    int msg_id_len = genMsgId(mid);
    ms::IMessage* msg_ack = _parser->createMsgAckPack(pack, uid, mid._bytes, msg_id_len);
    _msg_router->routeBack(msg_ack);

    IMessage* msg = pack->_msg;
    msg->_uid = uid;
    memcpy(msg->_id, mid._bytes, msg_id_len);

    int ret = _broker->dispatch(msg);
    if (ret == 0)
    {
        ms::releaseIMessage(msg);
    }
    else
    {
        if (_pending_msg_collector)
        {
            LOG(INFO) << "dispatch to ms failed, collect the message\n";
            _pending_msg_collector->collect(msg);
        }
        else
        {
            LOG(INFO) << "dispatch to ms failed, drop the message\n";
            ms::releaseIMessage(msg);
        }
    }

    return 0;
}

int ImMessageProcessor::processAuthPack(IMessagePack* pack)
{
    if (_parser->checkTimeStamp(pack) == -1)
        return -1;

    if (pack->_msg->_state != MSG_STATE)
        return -1;

    UID uid = _parser->getAuthPackUid(pack);
    if (uid != -1)
    {
        ms::IMessage* msg = _parser->createAuthPack(uid, pack);
        _msg_router->routeBack(msg);
        return 0;
    }

    return -1;
}

int ImMessageProcessor::process(ms::IMessage* msg)
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
        case MSG:
            ret = processMsgPack(pack);
            break;
        case TOUCH:
            ret = processTouchPack(pack);
            break;
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
        LOG(INFO) << "illeagle message, kick it...\n";
        processErrorPack(msg);
        return -1;
    }
}

}
