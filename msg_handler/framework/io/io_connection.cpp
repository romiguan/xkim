#include "io_connection.h"
#include "io_push_list.h"
#include "connection_cache.h"
#include "connection_register.h"

#include <unistd.h>
#include <event.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <new>
#include <glog/logging.h>

#define FRAME_MAX_SIZE 1024
#define READ_BUFFER_INCREAMENT_SIZE 64
#define READ_TIMEOUT 100
#define WRITE_TIMEOUT 30

namespace io
{

IoConnection::IoConnection():
    _rsock_state(SOCKET_RECV_FRAMING),
    _wsock_state(SOCKET_DATA_WRITE),
    _rbuf(new (std::nothrow) char [CONN_DEFAULT_READ_LEN]),
    _rsize(CONN_DEFAULT_READ_LEN),
    _rpos(0),
    _frame_size(0),
    _current_push(0),
    _rtimeout(-1),
    _wtimeout(-1),
    _destroy(0),
    _revent_flag(0),
    _wevent_flag(0)
{
    if (_rbuf == 0)
    {
        LOG(ERROR) << "connection construct error, abort\n";
        abort();
    }
}

IoConnection::IoConnection(int fd):
    _socket(fd),
    _rsock_state(SOCKET_RECV_FRAMING),
    _wsock_state(SOCKET_DATA_WRITE),
    _rbuf(new char [CONN_DEFAULT_READ_LEN]),
    _rsize(CONN_DEFAULT_READ_LEN),
    _rpos(0),
    _frame_size(0),
    _current_push(0),
    _rtimeout(-1),
    _wtimeout(-1),
    _destroy(0),
    _revent_flag(0),
    _wevent_flag(0)
{
    if (_rbuf == 0)
    {
        LOG(ERROR) << "connection construct error, abort\n";
        abort();
    }
}

IoConnection::~IoConnection()
{
}

void IoConnection::setReadFlag(short flag, int timeout)
{
    if (_revent_flag != 0)
        event_del(&_revent);

    _revent_flag = flag;

    if (_revent_flag == 0)
        return;

    event_set(&_revent, _socket, _revent_flag, IoConnection::readHandler, this);
    event_base_set(_event_base, &_revent);

    if (timeout > 0)
    {
        struct timeval tm = {timeout, 0};
        event_add(&_revent, &tm);
    }
    else
    {
        event_add(&_revent, 0);
    }

    return;
}

void IoConnection::setWriteFlag(short flag, int timeout)
{
    if (_wevent_flag == flag)
        return;

    if (_wevent_flag != 0)
        event_del(&_wevent);

    _wevent_flag = flag;

    if (_wevent_flag == 0)
        return;

    event_set(&_wevent, _socket, _wevent_flag, IoConnection::writeHandler, this);
    event_base_set(_event_base, &_wevent);

    if (timeout > 0)
    {
        struct timeval tm = {timeout, 0};
        event_add(&_wevent, &tm);
    }
    else
    {
        event_add(&_wevent, 0);
    }

    return;
}

void IoConnection::setWrite(bool flag)
{
    if (_wsock_state == SOCKET_CONNECTING)
        return;
    _setWrite(true);
}

void IoConnection::_setRead(bool flag)
{
    short event_flag = flag ? EV_READ|EV_PERSIST : 0;
    setReadFlag(event_flag, _rtimeout);
}

void IoConnection::_setWrite(bool flag)
{
    short event_flag = flag ? EV_WRITE|EV_PERSIST : 0;
    setWriteFlag(event_flag, _wtimeout);
}

void IoConnection::closeSocket()
{
    _destroy = 1;

    _setRead(false);
    _setWrite(false);

    ::close(_socket);
    _socket = -1;

    if (_current_push)
        this->pendingPushData(_current_push);
    _current_push = 0;

    whenClosed();

    if (_when_destroy)
        _when_destroy(this);
}

void IoConnection::resetSocket()
{
    _destroy = 1;

    _setRead(false);
    _setWrite(false);

    struct linger l = {1, 0};
    setsockopt(_socket, SOL_SOCKET, SO_LINGER, (void*)&l, sizeof(l));

    ::shutdown(_socket, SHUT_RDWR);
    ::close(_socket);
    _socket = -1;

    if (_current_push)
        this->pendingPushData(_current_push);
    _current_push = 0;

    whenClosed();

    if (_when_destroy)
        _when_destroy(this);
}

void IoConnection::readSocket()
{
    switch (_rsock_state)
    {
        case SOCKET_RECV_FRAMING:
        {
            while (true)
            {
                int fetch = ::recv(_socket, _rbuf+_rpos, _rsize-_rpos, 0);
                if (fetch > 0)
                {
                    _rpos += fetch;
                }
                else if (fetch == 0)
                {
                    //we think client closed
                    closeSocket();
                    return;
                }
                else
                {
                    switch (errno)
                    {
                        case EAGAIN:
                            break;
                        case EINTR:
                            continue;
                        default:
                            //any other error, reset conn
                            resetSocket();
                            break;
                        /*
                        default:
                        {
                            //the latest write may cause error: ETIMEDOUT,EHOSTUNREACH,ENETUNREACH
                            if (_rpos > 0 || errno == ETIMEDOUT || errno == EHOSTUNREACH || errno == ENETUNREACH)
                            {
                                //we have received some data,so we think client abnormally closed
                                //resetSocket();
                                //we should keep TIME_WAIT
                                closeSocket();
                            }
                            else
                            {
                                //the latest write may cause error: ECONNRESET, but we can't distinguish this situation
                                //we think client normally close
                                closeSocket();
                            }
                            return;
                        }
                        */
                    }
                }

                break;
            }

            //we have some data arrived
            if (_rpos < (int)sizeof(int))
            {
                //TODO,if we reset timeout
                return;
            }

            _frame_size = ntohl(*((unsigned int*)_rbuf)) + sizeof(int);
            if (_frame_size > FRAME_MAX_SIZE)
            {
                //frame too large, close
                resetSocket();
                return;
            }

            if ((int)_frame_size > _rsize)
            {
                int new_size = _frame_size + READ_BUFFER_INCREAMENT_SIZE;
                if (new_size > FRAME_MAX_SIZE)
                    new_size = FRAME_MAX_SIZE;
                char* new_buffer = new (std::nothrow) char [new_size];
                if (new_buffer == 0)
                {
                    //memory error,reset
                    resetSocket();
                    return;
                }

                memcpy(new_buffer, _rbuf, _rpos);
                delete [] _rbuf;

                _rbuf = new_buffer;
                _rsize = new_size;
            }

            _rsock_state = SOCKET_DATA_RECV;
        }
        case SOCKET_DATA_RECV:
        {
            int left = _frame_size-_rpos;
            while (left > 0)
            {
                int fetch = ::recv(_socket, _rbuf+_rpos, left, 0);
                if (fetch > 0)
                {
                    _rpos += fetch;
                    break;
                }
                else if (fetch == 0)
                {
                    closeSocket();
                    return;
                }
                else
                {
                    switch (errno)
                    {
                        case EINTR:
                            continue;
                        case EAGAIN:
                            return;
                        default:
                            resetSocket();
                            return;
                    }
                }
            }

            if (_rpos == (int)_frame_size)
            {
                int ret = this->whenReceivedFrame(_rbuf, _frame_size);
                if (ret == -1)
                {
                    //internal error, close
                    resetSocket();
                    return;
                }

                //接着读新请求
                _rpos = 0;
                _rsock_state = SOCKET_RECV_FRAMING;
            }
            else if (_rpos > (int)_frame_size)
            {
                char* b = _rbuf;
                int frame_size = _frame_size;
                int len = _rpos;
                int ret;
                while (len >= frame_size)
                {
                    ret = this->whenReceivedFrame(b, frame_size);
                    if (ret == -1)
                    {
                        //internal error, close
                        resetSocket();
                        return;
                    }

                    b += frame_size;
                    len -= frame_size;
                    if (len >= (int)sizeof(int))
                    {
                        frame_size = ntohl(*((unsigned int*)b)) + sizeof(int);
                    }
                    else
                    {
                        break;
                    }
                }

                memcpy(_rbuf, b, len);
                _rpos = len;
                _rsock_state = SOCKET_RECV_FRAMING;
            }

            break;
        }
        default:
        {
            LOG(ERROR) << "socket state error, abort\n";
            abort();
        }
    }
}

void IoConnection::writeSocket()
{
    switch (_wsock_state)
    {
        case SOCKET_CONNECTING:
        {
            int error = -1;
            socklen_t len = sizeof(error);
            getsockopt(_socket, SOL_SOCKET, SO_ERROR, &error, &len);
            if (error != 0)
            {
                LOG(INFO) << "connecting error\n";

                resetSocket();
                return;
            }

            _wsock_state = SOCKET_DATA_WRITE;
        }
        case SOCKET_DATA_WRITE:
        {
            if (_current_push == 0)
            {
                _current_push = getPushData();
                if (_current_push == 0)
                {
                    _setWrite(false);
                    _setRead(true);
                    return;
                }
            }

            if (_current_push->_msg->_close_conn)
            {
                //need closed by upstream logic
                resetSocket();
                return;
            }

            char* wbuf = _current_push->_wbuf + _current_push->_wpos;
            int left = _current_push->_wsize - _current_push->_wpos;

            while (left > 0)
            {
                int32_t put = ::send(_socket, wbuf, left, MSG_NOSIGNAL);
                if (put > 0)
                {
                    _current_push->_wpos += put;
                    left -= put;
                    break;
                }
                else if (put == -1)
                {
                    switch (errno)
                    {
                        case EINTR:
                            continue;
                        case EAGAIN:
                            return;
                        default:
                            resetSocket();
                            return;
                    }
                }
                else
                {
                    return;
                }
            }

            if (left == 0)
            {
                //TODO
                //we send msg to wan,but the peer may not receive
                //we just drop the msg, in the future may be use ack
                destroyMsgData(_current_push);
                _current_push = getPushData();
                if (_current_push == 0)
                {
                    _setWrite(false);
                    _setRead(true);
                    return;
                }
            }

            break;
        }
        default:
        {
            LOG(INFO) << "write state(" << _wsock_state << ") error\n";
            resetSocket();
            return;
        }
    }
}

void IoConnection::readHandler(int fd, short which, void* v)
{
    (void)fd;
    IoConnection* conn = (IoConnection*)v;
    if (conn->_destroy)
        return;

    if (which & EV_TIMEOUT)
    {
        conn->handleTimeout();
        return;
    }
    conn->readSocket();
}

void IoConnection::writeHandler(int fd, short which, void* v)
{
    (void)fd;
    IoConnection* conn = (IoConnection*)v;
    if (conn->_destroy)
        return;

    if (which & EV_TIMEOUT)
    {
        conn->handleTimeout();
        return;
    }
    conn->writeSocket();
}

void IoConnection::initialize(int fd,
        int io,
        uint64_t id,
        void* owner,
        struct event_base* event_base)
{
    _socket = fd;
    _io = io;
    _id = id;
    _owner = owner;
    _rsock_state = SOCKET_RECV_FRAMING;
    _wsock_state = SOCKET_DATA_WRITE;
    if (_rbuf == 0)
    {
        _rbuf = new char [CONN_DEFAULT_READ_LEN];
        if (_rbuf == 0)
        {
            LOG(ERROR) << "memory error, abort\n";
            abort();
        }

        _rsize = CONN_DEFAULT_READ_LEN;
    }
    _rpos = 0;
    _frame_size = 0;
    _current_push = 0;

    _destroy = 0;
    _revent_flag = 0;
    _wevent_flag = 0;

    _event_base = event_base;
    _when_destroy = 0;
}

}
