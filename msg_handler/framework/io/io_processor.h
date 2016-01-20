#ifndef __IO_PROCESSOR_H__
#define __IO_PROCESSOR_H__

#include "framework/ms/ms_message.h"

namespace io
{

class IoProcessor
{
    public:
        IoProcessor() {}
        virtual ~IoProcessor() {}

        virtual int process(ms::IMessage* msg) = 0;
};

}

#endif
