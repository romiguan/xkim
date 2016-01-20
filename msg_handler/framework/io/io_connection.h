#ifndef __IO_CONNECTION_H__
#define __IO_CONNECTION_H__

#include <stdlib.h>
#include <event.h>

#define CONN_DEFAULT_READ_LEN 1024

namespace io
{

struct IoPushData;
class IoConnection;
typedef void (*whenDestroy) (IoConnection* conn);

class IoConnection
{
    public:
        enum SOCK_STATE
        {
            SOCKET_CONNECTING = 1,
            SOCKET_DATA_WRITE,
            SOCKET_RECV_FRAMING,
            SOCKET_DATA_RECV,
        };

    public:
        void* _owner;
        uint64_t _id;

    public:
        IoConnection();
        IoConnection(int fd);
        virtual ~IoConnection();

        void initialize(int fd,
                int io,
                uint64_t  id,
                void* owner,
                struct event_base* event_base);
        void setIO(void* base) { _event_base = (struct event_base*)base; }
        void setRead(bool flag) { _setRead(flag); }
        void setWrite(bool flag);
        void setConnecting()
        {
            _wsock_state = SOCKET_CONNECTING;
            _setWrite(true);
        }
        void setReadTimeout(int timeout) { _rtimeout = timeout; }
        void setWriteTimeout(int timeout) { _wtimeout = timeout; }
        void setWhenDestroy(whenDestroy destroy) { _when_destroy = destroy; }

        uint64_t id() const { return _id; }
        void id(uint64_t id) { _id = id; }

    protected:
        void setReadFlag(short flag, int timeout);
        void setWriteFlag(short flag, int timeout);
        void _setWrite(bool flag);
        void _setRead(bool flag);
        void handleTimeout() { resetSocket(); }

        void readSocket();
        void writeSocket();

        void resetSocket();
        void closeSocket();

    protected:
        virtual int whenReceivedFrame(char* buf, int size) { return 0; }
        virtual int whenClosed() { return 0; }
        virtual IoPushData* getPushData() { return 0; }
        virtual void pendingPushData(IoPushData* data) { return; }

    private:
        static void readHandler(int fd, short which, void* v);
        static void writeHandler(int fd, short which, void* v);

    protected:
        int _socket;
        int _io;
        SOCK_STATE _rsock_state;
        SOCK_STATE _wsock_state;
        char* _rbuf;
        int _rsize;
        int _rpos;
        unsigned int _frame_size;

        IoPushData* _current_push;

        int _rtimeout;
        int _wtimeout;
        int _destroy;
        whenDestroy _when_destroy;

        short _revent_flag;
        short _wevent_flag;
        struct event _revent;
        struct event _wevent;
        struct event_base* _event_base;
};

}

#endif
