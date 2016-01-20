#ifndef __MS_MESSAGE_COLLECTOR_H__
#define __MS_MESSAGE_COLLECTOR_H__

namespace ms
{

struct IMessage;
class MsMessageCollectorCore
{
    public:
        MsMessageCollectorCore() {}
        virtual ~MsMessageCollectorCore() {}

        virtual void collect(IMessage* msg) = 0;
};

class MsMessageCollector
{
    public:
        MsMessageCollector();
        ~MsMessageCollector();

        void setCollector(MsMessageCollectorCore* c);
        void collect(IMessage* msg);
        void collectList(IMessage* msg);
        int run();

    private:
        class Impl;
        Impl* _core;
};

}

#endif
