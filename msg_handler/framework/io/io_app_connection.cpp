#include "io_app_connection.h"
#include "io_processor.h"
#include "ms/ms_message.h"

#include <glog/logging.h>

void switch_state_app_msg(void* v, int s)
{
    io::IoAppConnection* conn = (io::IoAppConnection*)v;
    conn->setState(s);
}

namespace io
{

IoAppConnection::IoAppConnection():
    IoConnection(),
    _state(0),
    _processor(0),
    _pending_message_collector(0)
{
}

IoAppConnection::~IoAppConnection()
{
}

void IoAppConnection::pushMsgData(ms::IMessage* msg)
{
    IoPushData* data = createPushData();
    data->_next = 0;
    data->_msg = msg;
    data->_freev = ms::releaseIMessage;

    data->_wbuf = msg->_b;
    data->_wsize = msg->_frame_len;
    data->_wpos = 0;

    _push_buffer.put(data);
}

int IoAppConnection::whenReceivedFrame(char* buf, int size)
{
    int* frame = (int*)buf;
    if (frame[1] == ms::magic)
    {
        IoSampling::Sample stat;
        _sampling.getSample(stat);

        /*
        fprintf(stderr, "s: %d(%d), m: %d(%d), h: %d(%d)\n", stat.sec_total,
                _second_limit,
                stat.min_total,
                _minute_limit,
                stat.hou_total,
                _hour_limit);
                */

        if (stat.sec_total > (unsigned int)_second_limit
                || stat.min_total > (unsigned int)_minute_limit
                || stat.hou_total > (unsigned int)_hour_limit)
            return -1;

        ms::IMessage* msg = ms::createIMessage(buf, size);
        msg->_state = _state;
        msg->_io = _io;
        msg->_conn = _id;
        msg->_type = ms::APP;
        gettimeofday(&msg->_gen_time, 0);

        _processor->process(msg);

        _sampling.sample();

        return 0;
    }

    return -1;
}

int IoAppConnection::whenClosed()
{
    if (_pending_data)
    {
        _push_buffer.putFirst(_pending_data);
        _pending_data = 0;
    }

    ms::IMessage* msg = _push_buffer.stealMsg();
    if (msg)
    {
        if (_pending_message_collector)
        {
            LOG(INFO) << "connection being closed, collect the pushlist message\n";
            _pending_message_collector->collect(msg);
            _push_buffer.shallowClear();
        }
        else
        {
            LOG(INFO) << "connection being closed, drop the pushlist message\n";
            _push_buffer.clear();
        }
    }

    return 0;
}

IoPushData* IoAppConnection::getPushData()
{
    return _push_buffer.get();
}

void IoAppConnection::pendingPushData(IoPushData* data)
{
    _pending_data = data;
}

void IoAppConnection::initialize(int fd,
        int io,
        uint64_t id,
        void* owner,
        struct event_base* event_base,
        IoProcessor* processor)
{
    IoConnection::initialize(fd, io, id, owner, event_base);
    _state = 0;
    _processor = processor;
    _pending_data = 0;
}

}
