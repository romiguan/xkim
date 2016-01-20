#ifndef __MESSAGE_PROXY_H__
#define __MESSAGE_PROXY_H__

#include "thrift/gen/Message.h"
#include "thrift/pilot/thrift_proxy.h"

using namespace ms;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace Pilot;

namespace im
{

class MessageProxy : public MessageIf 
{
    DECLARE_THRIFT_PROXY(Message);

    public:
        void dispatch(MessageResponse& _return, const MessageRequest& request)
        {
            INVOKE_AND_RETURN_WITHIN_3_ATTEMPTS(Message, dispatch, _return, request);
        }
};

}

#endif
