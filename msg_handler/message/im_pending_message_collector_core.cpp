#include "im_pending_message_collector_core.h"
#include "im_user_message_router.h"
#include "framework/ms/ms_message.h"

#include <stdio.h>

namespace im
{

ImPendingMessageCollectorCore::ImPendingMessageCollectorCore(ImUserMessageRouter* router):
    _router(router)
{
}

ImPendingMessageCollectorCore::~ImPendingMessageCollectorCore()
{
}

void ImPendingMessageCollectorCore::collect(ms::IMessage* msg)
{
    ms::IMessage* max_item = msg;
    ms::IMessage* item = msg->_next;
    while (item)
    {
        if (item->_conn > max_item->_conn)
            max_item = item;
        item = item->_next;
    }

    int ret = _router->failureRoute(msg->_uid, max_item->_conn, msg);
    if (ret == -1)
    {
        while (msg)
        {
            ms::IMessage* m = msg;
            msg = msg->_next;
            ms::releaseIMessage(m);
        }
    }
}

}
