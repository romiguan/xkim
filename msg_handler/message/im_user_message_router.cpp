#include "im_user_message_router.h"
#include "framework/io/io_channel.h"

#include <stdio.h>

#include <glog/logging.h>

extern uint64_t g_im_instance;

namespace im
{

ImUserMessageRouter::ImUserMessageRouter():
    _local(0)
{
}

ImUserMessageRouter::~ImUserMessageRouter()
{
}

ms::IMessage* ImUserMessageRouter::route(ms::IMessage* msg)
{
    io::IoChannel channel;
    _register_table_local.get(msg->_uid, channel);
    if (channel._uid != -1)
    {
        msg->_io = channel._io;
        msg->_conn = channel._conn;
        _local->route(msg);
        return 0;
    }

    return msg;
}

ms::IMessage* ImUserMessageRouter::routeList(ms::IMessage* msg)
{
    ms::IMessage* local_list = 0;
    ms::IMessage** local_trans = &local_list;
    ms::IMessage* pending_list = 0;
    ms::IMessage** pending_trans = &pending_list;

    io::IoChannel channel;
    while (msg)
    {
        _register_table_local.get(msg->_uid, channel);
        if (channel._uid != -1)
        {
            msg->_io = channel._io;
            msg->_conn = channel._conn;
            *local_trans = msg;
            local_trans = &(msg->_next);
            msg = msg->_next;
            *local_trans = 0;
            continue;
        }
        else
        {
            *pending_trans = msg;
            pending_trans = &(msg->_next);
            msg = msg->_next;
            *pending_trans = 0;
        }
    }

    if (local_list)
        _local->route(local_list);

    return pending_list;
}

int ImUserMessageRouter::failureRoute(UID uid, uint64_t maxConn, ms::IMessage* msg)
{
    io::IoChannel channel;
    _register_table_local.getOrUnset(uid, maxConn, channel);
    if (channel._uid != -1)
    {
        int new_io = channel._io;
        int new_conn = channel._conn;
        ms::IMessage* item = msg;
        while (item)
        {
            item->_io = new_io;
            item->_conn = new_conn;
            item = item->_next;
        }

        _local->route(msg);
    }

    return -1;
}

int ImUserMessageRouter::update(ms::IMessage* msg)
{
    io::IoChannel channel;
    channel._uid = msg->_uid;
    channel._host = g_im_instance;
    channel._io = msg->_io;
    channel._conn = msg->_conn;

    _register_table_local.set(msg->_uid, channel);

    return 0;
}

}
