#ifndef __IM_USER_MESSAGE_ROUTER_H__
#define __IM_USER_MESSAGE_ROUTER_H__

#include "im_user_channel_register_table_local.h"
#include "framework/ms/ms_message_router_local.h"
#include "framework/ms/ms_message.h"

namespace im
{

class ImUserMessageRouter
{
    public:
        ImUserMessageRouter();
        ~ImUserMessageRouter();

        void setLocalRouter(ms::MsMessageRouterLocal* router)
        {
            _local = router;
        }

        ms::IMessage* route(ms::IMessage* msg);
        ms::IMessage* routeList(ms::IMessage* msg);
        void routeBack(ms::IMessage* msg) { _local->route(msg); }
        int failureRoute(UID uid, uint64_t maxConn, ms::IMessage* msg);

        int update(ms::IMessage* ms);

    private:
        ImUserChannelRegisterTableLocal _register_table_local;
        ms::MsMessageRouterLocal* _local;
};

}

#endif
