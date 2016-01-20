#include "io_manager.h"
#include "io_accept.h"
#include "io_app_accept_service.h"
//#include "io_backend_accept_service.h"
#include "lib/util.h"
#include "lib/ini_file_reader.h"

#include <stdio.h>
#include <stdlib.h>

#include <glog/logging.h>

uint64_t g_im_instance;

namespace io
{

IoManager::IoManager():
    _io(0),
    _io_cluster(0)
    //_backend_io_cluster(0)
{
}

IoManager::~IoManager()
{
}

int IoManager::initialize(const char* config)
{
    _io = new (std::nothrow) IoAccept();
    if (_io == 0)
    {
        LOG(ERROR) << "memory error, abort\n";
        abort();
    }

    util::IniFileReader ini_reader(config);
    int app_port = ini_reader.IniGetIntValue("IO", "io_app_port", 0);
    if (app_port <= 0)
    {
        LOG(ERROR) << "[IO]::[io_port] error, abort\n";
        abort();
    }

    //g_im_instance = util::gethost("eth1");
    //g_im_instance = (g_im_instance << 32) | 9002;

    int io_count = 0;
    //TODO, crete other accept service
    if (_io_cluster)
    {
        _io->addService(new IoAppAcceptService(app_port, "", _io_cluster));
        ++io_count;
    }

    /*
    if (_backend_io_cluster)
    {
        _io->addService(new IoBackendAcceptService(9002, "", _backend_io_cluster));
        ++io_count;
    }
    */

    return io_count > 0 ? 0 : -1;
}

int IoManager::run()
{
    return _io->run();
}

int IoManager::join()
{
    return _io->join();
}

}

