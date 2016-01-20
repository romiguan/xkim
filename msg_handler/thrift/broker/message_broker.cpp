#include "message_broker.h"
#include "message_broker_processor.h"
#include "broker_worker_cluster.h"
#include "framework/ms/ms_message.h"
#include "framework/ms/ms_worker_cluster.h"
#include "framework/lib/posix_thread_factory.h"
#include "framework/lib/ini_file_reader.h"
#include "thrift/gen/Message.h"
#include "thrift/pilot/zookeeper_client.h"
#include "thrift/pilot/watcher_action.h"

#include <assert.h>
#include <arpa/inet.h>

//thrift
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/Thread.h>
#include <thrift/concurrency/PosixThreadFactory.h>

#include <glog/logging.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

uint64_t g_broker_instance;

namespace im
{

namespace
{

using Pilot::ZookeeperClient;
using Pilot::RegServiceWatcherAction;
using Pilot::ServiceWatcherAction;

uint64_t genInstance(const std::string& host, int port)
{
    unsigned int ip_binary;
    int ret = inet_pton(AF_INET, host.c_str(), &ip_binary);
    if (ret == 1)
    {
        uint64_t id = ip_binary;
        id = (id << 32) | port;
        return id;
    }
    else
    {
        return 0;
    }
}

ZookeeperClient* init_rpc_client(std::string& zk)
{
    ZookeeperClient* zk_clientp = new ZookeeperClient;
    assert(zk_clientp != 0);

    zk_clientp->set_host(zk);
    zk_clientp->set_recv_timeout(3);

    RegServiceWatcherAction* watcher = new RegServiceWatcherAction(new ServiceWatcherAction());
    assert(watcher != 0);
    watcher->set_owner(zk_clientp);
    zk_clientp->set_watcher(watcher);
    return zk_clientp;
}

ZookeeperClient* init_rpc_server(const char* conf)
{
    util::IniFileReader ini_reader(conf);

    ZookeeperClient* zk_clientp = new ZookeeperClient;
    assert(zk_clientp != 0);

    std::string zk = ini_reader.IniGetStrValue("BROKER", "broker_rpc_local_zk", "");
    assert(zk.empty() == false);
    zk_clientp->set_host(zk);
    zk_clientp->set_recv_timeout(3);

    std::string server_path = ini_reader.IniGetStrValue("BROKER", "broker_rpc_local_path", "");
    assert(server_path.empty() == false);
    std::string server_protocol("TBinaryProtocol");
    int server_port = ini_reader.IniGetIntValue("BROKER", "broker_port", 0);

    std::vector<ServerCluster*> server;
    ServerCluster* clusterp = new ServerCluster(server_path, server_protocol, server_port);
    assert(clusterp != NULL);
    clusterp->set_zookeeper_client(zk_clientp);
    server.push_back(clusterp);

    RegServiceWatcherAction* watcher = new RegServiceWatcherAction(server);
    assert(watcher != NULL);
    watcher->set_owner(zk_clientp);
    zk_clientp->set_watcher(watcher);

    g_broker_instance = genInstance(clusterp->get_server_ip(), server_port);

    return zk_clientp;
}

}

class MessageHandler : public ms::MessageIf
{
    public:
        MessageHandler(ms::MsWorkerCluster* workers):
            _workers(workers)
        {}
        virtual ~MessageHandler() {}

        virtual void dispatch(ms::MessageResponse& _return, const ms::MessageRequest& request)
        {
            if (request.msg.empty() == false)
            {
                const char* msg_content = request.msg.c_str();
                int size = request.msg.size();
                ms::IMessage* msg = ms::createIMessage(size);
                msg->_priority = 1;
                memcpy(msg->_data, msg_content, size);

                _workers->schedule(msg);
                _return.__set_code(ms::ResponseCode::OK);
            }
            else
            {
                _return.__set_code(ms::ResponseCode::ERROR);
            }
            return;
        }

    private:
        ms::MsWorkerCluster* _workers;
};

class ServerRun : public util::Runnable
{
    public:
        ServerRun(TNonblockingServer* server):
            _server(server)
        {
        }

        virtual ~ServerRun() {}

        virtual int run()
        {
            _server->serve();
            return 0;
        }

    private:
        TNonblockingServer* _server;
};

int MessageBroker::initialize(const char* conf)
{
    strcpy(_conf, conf);

    util::IniFileReader ini_reader(conf);
    int conn_num = ini_reader.IniGetIntValue("BROKER", "broker_rpc_conn_num", 0);
    if (conn_num <= 0)
        conn_num = 32;
    int broker_rpc_conn_timeout = ini_reader.IniGetIntValue("BROKER", "broker_rpc_conn_timeout", 0);
    if (broker_rpc_conn_timeout <= 0)
        broker_rpc_conn_timeout = 10;
    int broker_rpc_read_timeout = ini_reader.IniGetIntValue("BROKER", "broker_rpc_read_timeout", 0);
    if (broker_rpc_read_timeout <= 0)
        broker_rpc_read_timeout = 30;
    int broker_rpc_write_timeout = ini_reader.IniGetIntValue("BROKER", "broker_rpc_write_timeout", 0);
    if (broker_rpc_write_timeout <= 0)
        broker_rpc_write_timeout = 30;

    _port = ini_reader.IniGetIntValue("BROKER", "broker_port", 0);
    if (_port <= 0)
    {
        LOG(ERROR) << "[BROKER]::[broker_port] error, abort\n";
        abort();
    }

    _worker_num = ini_reader.IniGetIntValue("BROKER", "broker_worker_num", 0);
    if (_worker_num <= 0)
        _worker_num = 1;

    _io_num = ini_reader.IniGetIntValue("BROKER", "broker_io_num", 0);
    if (_io_num <= 0)
        _io_num = 1;

    std::string zk = ini_reader.IniGetStrValue("BROKER", "broker_rpc_remote_zk", "");
    assert(zk.empty() == false);
    ZookeeperClient* rpc_client = init_rpc_client(zk);
    std::string path = ini_reader.IniGetStrValue("BROKER", "broker_rpc_remote_path", "");
    assert(path.empty() == false);
    PooledClient(Message)::fixpath(path);
    _service = PooledClient(Message)::instance_simple(rpc_client,
            path,
            conn_num,
            broker_rpc_conn_timeout,
            broker_rpc_read_timeout,
            broker_rpc_write_timeout);
    rpc_client->open();
    sleep(1);
    return 0;
}

int MessageBroker::dispatch(ms::IMessage* msg)
{
    try
    {
        ms::MessageRequest request;
        request.__set_msh_id(g_broker_instance);
        request.msg_id.assign(msg->_id, sizeof(msg->_id));
        request.__isset.msg_id = true;
        request.msg.assign(msg->_data, msg->_data_len);
        request.__isset.msg = true;

        ms::MessageResponse response;
        _service->acquire()->dispatch(response, request);
        if (response.code == ms::ResponseCode::OK)
        {
            return 0;
        }
        else
        {
            //TODO invalid message
            //drop it
            LOG(INFO) << "dispatch message error, drop it\n";
            return 0;
        }
    }
    catch (const TZException& e)
    {
        switch (e.tze_insight_no())
        {
            case TZException::TZE_NOSERVICE:
                LOG(INFO) << "dispatch message exception: no service\n";
                break;
            case TZException::TZE_NET_TIMEOUT:
                LOG(INFO) << "dispatch message exception: timeout\n";
                break;
            case TZException::TZE_APP:
                LOG(INFO) << "dispatch message exception: peer throw\n";
                break;
            default:
                LOG(INFO) << "dispatch message exception: other\n";
                break;
        }
    }
    catch (...)
    {
        LOG(INFO) << "dispatch message exception: unknown\n";
    }

    return -1;
}

void MessageBroker::run()
{
    LOG(INFO) << "message broker running ...\n";

    MessageBrokerProcessor* processor = new MessageBrokerProcessor();
    processor->setMessageRouter(_msg_router);
    processor->setPendingMessageCollector(_pending_msg_collector);
    processor->setMessageParser(new im::ImMessageParser());

    BrokerWorkerCluster* workers = new BrokerWorkerCluster(_worker_num, processor);
    workers->run();

    boost::shared_ptr<MessageHandler> handler(new MessageHandler(workers));
    boost::shared_ptr<TProcessor> thrift_processor(new MessageProcessor(handler));
    boost::shared_ptr<TProtocolFactory> protocol_factory(new TBinaryProtocolFactory());
    TNonblockingServer* server = new TNonblockingServer(thrift_processor, protocol_factory, _port);
    server->setNumIOThreads(_io_num);

    util::PosixThreadFactory ptf;
    ptf.setPolicy(util::PosixThreadFactory::OTHER);
    ptf.setStackSize(10);
    ptf.setDetached(false);

    boost::shared_ptr<util::Runnable> core(new ServerRun(server));
    _thread = ptf.newThread(core);
    _thread->run();

    usleep(1000);

    ZookeeperClient* rpc_server = init_rpc_server(_conf);
    rpc_server->open();
}

void MessageBroker::join()
{
    _thread->join();
}

}
