#ifndef __IM_APP_MESSAGE_WORKER_CORE_H__
#define __IM_APP_MESSAGE_WORKER_CORE_H__

#include "framework/ms/ms_message_priority_queue.h"
#include "framework/ms/ms_message_processor.h"
#include "framework/ms/ms_worker_core.h"

namespace im
{

class ImAppMessageWorkerCore : public ms::MsWorkerCore
{
    public:
        ImAppMessageWorkerCore(ms::MsMessageProcessor* processor, ms::MsMessagePriorityQueue* queue);
        virtual ~ImAppMessageWorkerCore();

        virtual int run();

    private:
        ms::MsMessageProcessor* _processor;
        ms::MsMessagePriorityQueue* _queue;
};

}

#endif
