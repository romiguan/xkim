#include "broker_message_core.h"

#include <assert.h>

namespace im
{

BrokerMessageWorkerCore::BrokerMessageWorkerCore(ms::MsMessageProcessor* processor, ms::MsMessageBlockQueue* queue):
    _processor(processor),
    _queue(queue)
{
    assert(_processor);
    assert(_queue);
}

BrokerMessageWorkerCore::~BrokerMessageWorkerCore()
{
}

int BrokerMessageWorkerCore::run()
{
    assert(_processor);
    assert(_queue);

    int count = 0;
    while (true)
    {
        ++count;
        ms::IMessage* msg = (ms::IMessage*)_queue->get(1);
        _processor->process(msg);
        //交给processor处理，不需要释放
        //releaseIMessage(v);
    }

    return 0;
}

}
