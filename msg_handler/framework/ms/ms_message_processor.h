#ifndef __MS_MESSAGE_PROCESSOR_H__
#define __MS_MESSAGE_PROCESSOR_H__

namespace ms
{

struct IMessage;
class MsMessageProcessor
{
    public:
        MsMessageProcessor() {}
        virtual ~MsMessageProcessor() {}

        virtual int process(IMessage* msg) = 0;
};

}

#endif
