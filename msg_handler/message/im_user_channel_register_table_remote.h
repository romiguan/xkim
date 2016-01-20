#ifndef __IM_USER_CHANNEL_REGISTER_TABLE_REMOTE_H__
#define __IM_USER_CHANNEL_REGISTER_TABLE_REMOTE_H__

#include "framework/io/io_channel.h"

#include <stdint.h>

namespace im
{

class ImUserChannelRegisterTableRemote
{
    public:
        ImUserChannelRegisterTableRemote();
        ~ImUserChannelRegisterTableRemote();

        uint64_t get(UID uid);
        void set(UID uid, uint64_t);
};

}

#endif
