#ifndef __IM_AUTH_MESSAGE_WORKER_CORE_H__
#define __IM_AUTH_MESSAGE_WORKER_CORE_H__

#include "framework/ms/ms_message_block_queue.h"
#include "framework/ms/ms_message_processor.h"
#include "framework/ms/ms_worker_core.h"

namespace im
{

class ImAuthMessageWorkerCore : public ms::MsWorkerCore
{
    public:
        ImAuthMessageWorkerCore(ms::MsMessageProcessor* processor, ms::MsMessageBlockQueue* queue);
        virtual ~ImAuthMessageWorkerCore();

        virtual int run();

    private:
        ms::MsMessageProcessor* _processor;
        ms::MsMessageBlockQueue* _queue;
};

}

#endif
