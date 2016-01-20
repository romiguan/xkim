#ifndef __ACCEPT_SERVICE_H__
#define __ACCEPT_SERVICE_H__

namespace io
{

class AcceptService
{
    public:
        AcceptService() {}

        explicit AcceptService(int fd):
            _listen_socket(fd),
            _io(0)
        {
        }

        virtual ~AcceptService()
        {
        }

        //fd: new connection fd
        virtual int run(int fd) = 0;

    public:
        int _listen_socket;
        void* _io;
};

}

#endif
