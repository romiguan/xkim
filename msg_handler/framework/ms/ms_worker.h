#ifndef __MS_WORKER_H__
#define __MS_WORKER_H__

namespace ms 
{

class MsWorkerCore;
class MsWorker
{
    public:
        MsWorker(int id, MsWorkerCore* core);
        virtual ~MsWorker();

        int run();
        int stop();

    private:
        class Impl;
        Impl* _impl;
};

}

#endif
