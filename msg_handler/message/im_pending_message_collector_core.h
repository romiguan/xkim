#ifndef __IM_PEDING_MESSAGE_COLLECTOR_CORE_H__
#define __IM_PEDING_MESSAGE_COLLECTOR_CORE_H__

#include "framework/ms/ms_message_collector.h"

namespace im
{

class ImUserMessageRouter;
class ImPendingMessageCollectorCore : public ms::MsMessageCollectorCore
{
    public:
        explicit ImPendingMessageCollectorCore(ImUserMessageRouter* router);
        virtual ~ImPendingMessageCollectorCore();

        virtual void collect(ms::IMessage* msg);

    private:
        ImUserMessageRouter* _router;
};

}

#endif
