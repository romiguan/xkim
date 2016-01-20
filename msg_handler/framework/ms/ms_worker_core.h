#ifndef __MS_WORKER_CORE_H__
#define __MS_WORKER_CORE_H__

namespace ms
{

class MsWorkerCore
{
    public:
        MsWorkerCore() {}
        virtual ~MsWorkerCore() {}

        virtual int run() = 0;
};

}

#endif
