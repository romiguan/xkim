#include "im_app_message_core.h"

#include <assert.h>
#include <stdio.h>

namespace im
{

ImAppMessageWorkerCore::ImAppMessageWorkerCore(ms::MsMessageProcessor* processor, ms::MsMessagePriorityQueue* queue):
    _processor(processor),
    _queue(queue)
{
    assert(_processor);
    assert(_queue);
}

ImAppMessageWorkerCore::~ImAppMessageWorkerCore()
{
}

int ImAppMessageWorkerCore::run()
{
    assert(_processor);
    assert(_queue);

    int count = 0;
    while (true)
    {
        ++count;
        ms::IMessage* msg = (ms::IMessage*)_queue->get();
        _processor->process(msg);
    }

    return 0;
}

}
