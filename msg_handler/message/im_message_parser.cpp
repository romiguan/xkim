#include "im_message_parser.h"
#include "im_message_define.h"
#include "framework/lib/util.h"

#include <stdio.h>
#include <time.h>

#include <glog/logging.h>

namespace im
{

namespace
{

std::string empty_token;

}

IMessagePack* createIMessagePack(ms::IMessage* msg)
{
    //这里用malloc遇到一个坑,malloc不会调用构造函数,导致后面pack->_data.ParseFromArray() coredown
    IMessagePack* pack = new (std::nothrow) IMessagePack;
    if (pack == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    pack->_msg = msg;
    return pack;
}

void releaseIMessagePack(IMessagePack* pack)
{
    delete pack;
}

IMessagePackType ImMessageParser::getMessagePackType(IMessagePack* pack)
{
    switch (pack->_data.type())
    {
        case IM_UNKNOW:
            return UNKNOW;
        case IM_TOUCH:
            return TOUCH;
        case IM_MSG:
            return MSG;
        case IM_MSG_ACK:
            return MSG_ACK;
        case IM_AUTH:
            return AUTH;
        default:
            return UNKNOW;
    }
}

int ImMessageParser::checkTimeStamp(IMessagePack* pack)
{
    ImPack& data = pack->_data;
    int pack_ts = data.has_ts() ? data.ts() : 0;
    int now_ts = pack->_msg->_gen_time.tv_sec - TIMESTAMP_20150101000000;
    return (now_ts-pack_ts) > TIMESTAMP_GAP ? -1 : 0;
}

int ImMessageParser::getTouchPackUid(IMessagePack* pack)
{
    const ImPackContent& data = pack->_data.content();
    const ImTouch& touch = data.touch();
    return (data.has_touch() && touch.has_uid()) ? touch.uid() : -1;
}

ms::IMessage* ImMessageParser::createTouchAckPack(UID uid, IMessagePack* pack)
{
    ImPack& data = pack->_data;
    data.Clear();

    data.set_type(IM_TOUCH_ACK);
    data.set_ts(time(NULL) - TIMESTAMP_20150101000000);
    ImPackContent* content = data.mutable_content();
    content->mutable_touch_rep()->set_uid(uid);

    ms::IMessage* msg = pack->_msg;
    pack->_msg = 0;

    int size = data.ByteSize();
    ms::fixIMessage(msg, size);
    msg->_uid = uid;
    msg->_need_collect = 0;
    data.SerializeToArray(msg->_data, size);
    return msg;
}

int ImMessageParser::getAuthPackUid(IMessagePack* pack)
{
    const ImPackContent& data = pack->_data.content();
    const ImAuthRequest& auth = data.auth_req(); 
    return data.has_auth_req() && auth.has_uid() ? auth.uid() : -1;
}

const std::string& ImMessageParser::getAuthPackToken(IMessagePack* pack)
{
    const ImPackContent& data = pack->_data.content();
    const ImAuthRequest& auth = data.auth_req(); 
    return data.has_auth_req() && auth.has_token() ? auth.token() : empty_token;
}

ms::IMessage* ImMessageParser::createAuthPack(UID uid, IMessagePack* pack)
{
    ImPack& data = pack->_data;
    std::string token = data.content().auth_req().token();
    data.Clear();

    data.set_type(IM_AUTH_RESPONSE);
    data.set_ts(time(NULL) - TIMESTAMP_20150101000000);
    ImPackContent* content = data.mutable_content();
    content->mutable_auth_rep()->set_token(token);
    content->mutable_auth_rep()->set_auth_success(true);

    ms::IMessage* msg = pack->_msg;
    pack->_msg = 0;

    int size = data.ByteSize();
    ms::fixIMessage(msg, size);
    msg->_uid = uid;
    msg->_need_collect = 0;
    data.SerializeToArray(msg->_data, size);
    return msg;
}

int ImMessageParser::getMsgPackUser(IMessagePack* pack)
{
    const ImPackContent& data = pack->_data.content();
    const ImMsg& msg_data = data.msg();
    return (data.has_msg() && msg_data.has_uid()) ? msg_data.uid() : -1;
}

ms::IMessage* ImMessageParser::createMsgAckPack(IMessagePack* pack,
        UID uid,
        const unsigned char* msg_id,
        int len)
{
    ImPack& incoming = pack->_data;
    const ImPackContent& data = incoming.content();
    const ImMsg& msg_data = data.msg();
    int seq = msg_data.seq();
    int type = msg_data.type() == IM_REQUEST ? ms::APP_REQUEST : ms::APP_REPLY;
    pack->_type = type;
    if (type == ms::APP_REQUEST)
    {
        ImMsg* mutalbe_msg = incoming.mutable_content()->mutable_msg();
        mutalbe_msg->set_msg_id(msg_id, len);

        int size = incoming.ByteSize();
        ms::IMessage* msg = ms::fixIMessage(pack->_msg, size);
        incoming.SerializeToArray(msg->_data, size);
    }

    ImPack ret;
    ret.set_type(IM_MSG_ACK);
    ret.set_ts(time(NULL) - TIMESTAMP_20150101000000);
    ImPackContent* content = ret.mutable_content();
    ImMsgAck* ack = content->mutable_msg_ack();

    ack->set_msg_id(msg_id, len);
    ack->set_seq(seq);

    ms::IMessage* in_msg = pack->_msg;
    int size = ret.ByteSize();
    ms::IMessage* msg = ms::createIMessage(size);
    msg->_uid = uid;
    msg->_io = in_msg->_io;
    msg->_conn = in_msg->_conn;
    msg->_need_collect = 0;
    ret.SerializeToArray(msg->_data, size);
    return msg;
}

int ImMessageParser::getMsgPackTargetUserCount(IMessagePack* pack)
{
    ImPack& incoming = pack->_data;
    const ImMsg& msg_pack = incoming.content().msg();
    return msg_pack.target_uid_size();
}

int ImMessageParser::getMsgPackTargetUser(IMessagePack* pack, int* uid, int count)
{
    ImPack& incoming = pack->_data;
    const ImMsg& msg_pack = incoming.content().msg();
    int target_uid_count = msg_pack.target_uid_size();
    if (target_uid_count > count)
        target_uid_count = count;

    for (int i = 0; i < target_uid_count; ++i)
        uid[i] = msg_pack.target_uid(i);
    return target_uid_count;
}

ms::IMessage* ImMessageParser::createLocalMsgPack(IMessagePack* pack)
{
#define TARGET_MAX 1024
    static __thread UID* target_uid = 0;

    if (expect_false(target_uid == 0))
    {
        target_uid = (UID*)malloc(sizeof(UID)*TARGET_MAX);
        if (target_uid == 0)
        {
            LOG(ERROR) << "memory error, abort\n";
            abort();
        }
    }

    ImPack& incoming = pack->_data;
    const ImMsg& msg_pack = incoming.content().msg();
    int target_uid_count = msg_pack.target_uid_size();
    if (target_uid_count <= 0)
        return 0;

    for (int i = 0; i < target_uid_count; ++i)
        target_uid[i] = msg_pack.target_uid(i);

    ImMsg* mutalbe_msg = incoming.mutable_content()->mutable_msg();
    mutalbe_msg->clear_target_uid();
    mutalbe_msg->add_target_uid(target_uid[0]);

    ms::IMessage* req_msg = pack->_msg;
    pack->_msg = 0;

    int size = incoming.ByteSize();
    ms::fixIMessage(req_msg, size);
    req_msg->_uid = target_uid[0];
    req_msg->_need_collect = 1;
    req_msg->_type = pack->_type;
    req_msg->_from = ms::BACKEND;
    req_msg->_to = ms::APP;
    incoming.SerializeToArray(req_msg->_data, size);

    ms::IMessage* trans = req_msg;;
    for (int i = 1; i < target_uid_count; ++i)
    {
        mutalbe_msg->clear_target_uid();
        mutalbe_msg->add_target_uid(target_uid[i]);

        size = incoming.ByteSize();
        ms::IMessage* msg = ms::createIMessage(size);
        msg->_uid = target_uid[i];
        msg->_need_collect = 1;
        msg->_type = pack->_type;
        msg->_from = ms::BACKEND;
        msg->_to = ms::APP;
        incoming.SerializeToArray(msg->_data, size);
        trans->_next = msg;
        trans = msg;
    }

    return req_msg;
}

ms::IMessage* ImMessageParser::createLocalMsgPack(IMessagePack* pack,
        UID* uid,
        int count,
        const unsigned char* msg_id,
        int len)
{
    ImPack& incoming = pack->_data;
    ImMsg* mutalbe_msg = incoming.mutable_content()->mutable_msg();
    mutalbe_msg->set_msg_id(msg_id, len);

    ms::IMessage req_msg;
    req_msg._next = 0;

    ms::IMessage* trans = &req_msg;;
    for (int i = 0; i < count; ++i)
    {
        mutalbe_msg->clear_target_uid();
        mutalbe_msg->add_target_uid(uid[i]);

        int size = incoming.ByteSize();
        ms::IMessage* msg = ms::createIMessage(size);
        msg->_uid = uid[i];
        msg->_need_collect = 1;
        msg->_type = pack->_type;
        msg->_from = ms::APP;
        msg->_to = ms::APP;
        incoming.SerializeToArray(msg->_data, size);
        trans->_next = msg;
        trans = msg;
    }

    return req_msg._next;;
}

ms::IMessage* ImMessageParser::createRemoteMsgPack(IMessagePack* pack,
        UID* uid,
        int count,
        const unsigned char* msg_id,
        int len)
{
    ImPack& incoming = pack->_data;
    ImMsg* mutalbe_msg = incoming.mutable_content()->mutable_msg();

    mutalbe_msg->clear_target_uid();
    for (int i = 0; i < count; ++i)
        mutalbe_msg->add_target_uid(uid[i]);
    mutalbe_msg->set_msg_id(msg_id, len);

    int size = incoming.ByteSize();
    ms::IMessage* msg = ms::createIMessage(size);
    msg->_need_collect = 1;
    msg->_type = pack->_type;
    msg->_from = ms::APP;
    msg->_to = ms::BACKEND;
    incoming.SerializeToArray(msg->_data, size);
    return msg;
}

ms::IMessage* ImMessageParser::createPendingMsgPack(IMessagePack* pack,
        UID uid,
        const unsigned char* msg_id,
        int len)
{
    ImPack& incoming = pack->_data;
    ImMsg* mutalbe_msg = incoming.mutable_content()->mutable_msg();

    mutalbe_msg->clear_target_uid();
    mutalbe_msg->add_target_uid(uid);
    mutalbe_msg->set_msg_id(msg_id, len);

    int size = incoming.ByteSize();
    ms::IMessage* msg = ms::createIMessage(size);
    msg->_uid = uid;
    msg->_need_collect = 1;
    msg->_type = pack->_type;
    msg->_from = ms::APP;
    msg->_to = ms::UNKNOWN;
    incoming.SerializeToArray(msg->_data, size);
    return msg;
}

IMessagePack* ImMessageParser::parse(ms::IMessage* msg)
{
    IMessagePack* pack = createIMessagePack(msg);
    ImPack& data = pack->_data;
    bool ret = data.ParseFromArray(msg->_data, msg->_data_len);

    if (ret && data.has_type() && data.has_content())
    {
        return pack;
    }
    else
    {
        releaseIMessagePack(pack);
        return 0;
    }
}

}
