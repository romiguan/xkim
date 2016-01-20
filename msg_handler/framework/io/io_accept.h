#ifndef __IO_ACCEPT_H__
#define __IO_ACCEPT_H__

namespace io
{

class AcceptService;
class IoAccept
{
    public:
        IoAccept();
        virtual ~IoAccept();

        //add listen socket
        int addService(AcceptService* s);

        int run();
        int stop();
        int join();

    private:
        class Impl;
        Impl* _core;
};

}

#endif
