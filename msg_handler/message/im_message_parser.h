#ifndef __IM_MESSAGE_PARSER_H__
#define __IM_MESSAGE_PARSER_H__

#include "framework/io/io_channel.h"
#include "framework/ms/ms_message.h"
#include "proto/im.pb.h"

#include <string>

namespace im
{

enum IMessagePackType
{
    UNKNOW = 0,
    TOUCH,
    MSG,
    MSG_ACK,
    AUTH,
};

struct IMessagePack
{
    ms::IMessage* _msg;
    ImPack _data;
    int _type;          // request/reply
};

class ImMessageParser
{
    public:
        ImMessageParser() {}
        virtual ~ImMessageParser() {}

        //主要得到当前用户的uid，目标用户的 uid集合，消息内容
        IMessagePack* parse(ms::IMessage* msg);
        IMessagePackType getMessagePackType(IMessagePack* pack);

        int checkTimeStamp(IMessagePack* pack);

        int getTouchPackUid(IMessagePack* pack);
        ms::IMessage* createTouchAckPack(UID uid, IMessagePack* pack);

        int getMsgPackUser(IMessagePack* pack);

        ms::IMessage* createMsgAckPack(IMessagePack* pack,
                UID uid,
                const unsigned char* msg_id,
                int len);

        int getMsgPackTargetUserCount(IMessagePack* pack);
        int getMsgPackTargetUser(IMessagePack* pack, int* uid, int count);

        int getAuthPackUid(IMessagePack* pack);
        const std::string& getAuthPackToken(IMessagePack* pack);
        ms::IMessage* createAuthPack(UID uid, IMessagePack* pack);

        /*
         * 由backend processor调用,生成路由到本地的消息
         */
        ms::IMessage* createLocalMsgPack(IMessagePack* pack);

        /*
         * 由processor调用,生成路由到本地的消息
         */
        ms::IMessage* createLocalMsgPack(IMessagePack* pack,
                UID* uid,
                int count,
                const unsigned char* msg_id,
                int len);

        /*
         * 由processor调用,生成路由到backend的消息
         */
        ms::IMessage* createRemoteMsgPack(IMessagePack* pack,
                UID* uid,
                int count,
                const unsigned char* msg_id,
                int len);

        ms::IMessage* createPendingMsgPack(IMessagePack* pack,
                UID uid,
                const unsigned char* msg_id,
                int len);
};

IMessagePack* createIMessagePack(ms::IMessage* msg);
void releaseIMessagePack(IMessagePack* pack);

}

#endif
