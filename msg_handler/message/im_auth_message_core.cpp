#include "im_auth_message_core.h"

#include <assert.h>
#include <stdio.h>

namespace im
{

ImAuthMessageWorkerCore::ImAuthMessageWorkerCore(ms::MsMessageProcessor* processor, ms::MsMessageBlockQueue* queue):
    _processor(processor),
    _queue(queue)
{
    assert(_processor);
    assert(_queue);
}

ImAuthMessageWorkerCore::~ImAuthMessageWorkerCore()
{
}

int ImAuthMessageWorkerCore::run()
{
    assert(_processor);
    assert(_queue);

    int count = 0;
    while (true)
    {
        ++count;
        ms::IMessage* msg = (ms::IMessage*)_queue->get(1);
        _processor->process(msg);
    }

    return 0;
}

}
