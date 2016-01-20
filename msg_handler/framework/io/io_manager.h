#ifndef __IO_MANAGER_H__
#define __IO_MANAGER_H__

namespace io
{

class IoAccept;
class IoAppCluster;
//class IoBackendCluster;
class IoManager
{
    public:
        IoManager();
        ~IoManager();

        void setIOAppCluster(IoAppCluster* cluster) { _io_cluster = cluster; }
        //void setIoBackendCluster(IoBackendCluster* cluster) { _backend_io_cluster = cluster; }
        int initialize(const char* conf);
        int run();
        int join();

    private:
        IoAccept* _io;
        IoAppCluster* _io_cluster;
        //IoBackendCluster* _backend_io_cluster;
};

}

#endif
