#ifndef __BROKER_MESSSAGE_WORKER_CORE_H__
#define __BROKER_MESSSAGE_WORKER_CORE_H__

#include "framework/ms/ms_message_block_queue.h"
#include "framework/ms/ms_message_processor.h"
#include "framework/ms/ms_worker_core.h"

namespace im
{

class BrokerMessageWorkerCore : public ms::MsWorkerCore
{
    public:
        BrokerMessageWorkerCore(ms::MsMessageProcessor* processor, ms::MsMessageBlockQueue* queue);
        virtual ~BrokerMessageWorkerCore();

        virtual int run();

    private:
        ms::MsMessageProcessor* _processor;
        ms::MsMessageBlockQueue* _queue;
};

}

#endif
