#include "im_user_channel_register_table_local.h"

#include <glog/logging.h>

namespace im
{

ImUserChannelRegisterTableLocal::ImUserChannelRegisterTableLocal()
{
    if (pthread_mutex_init(&_mutex, 0) != 0)
    {
        LOG(ERROR) << "pthread_mutex_init error, abort\n";
        abort();
    }
}

ImUserChannelRegisterTableLocal::~ImUserChannelRegisterTableLocal()
{
    pthread_mutex_destroy(&_mutex);
}

}
