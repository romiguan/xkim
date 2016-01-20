#include "framework/io/io_manager.h"
#include "framework/io/io_app_cluster.h"

#include "framework/ms/ms_message_router_local.h"
#include "framework/ms/ms_message_collector.h"

#include "message/im_message_dispatcher.h"
#include "message/im_user_message_router.h"
#include "message/im_pending_message_collector_core.h"

#include "thrift/broker/message_broker.h"
#include "thrift/broker/message_broker_processor.h"

#include <execinfo.h>
#include <stdio.h>
#include <signal.h>

#include <string>

#include <glog/logging.h>

static void lxPrintStack(FILE *fp)
{
    if (fp == NULL)
    {
        return;
    }

    void *array[64]; // 64 stacks at most
    size_t size = backtrace(array, sizeof(array)/sizeof(array[0]));
    if (size > 0 && size < 64)
    {
        char ** stackLog = backtrace_symbols(array, size);
        if(stackLog)
        {
            for (size_t i = 0; i < size; i++)
            {
                fprintf(fp,"%s\n", stackLog[i]);
            }
            fflush(fp);
        }

        free(stackLog);
    }
}

static void printStack(int sig)
{
    pid_t cur_pid = getpid();
    char pid_str[64];
    snprintf(pid_str, 64, "%d", cur_pid);
    std::string coreLogFile = "./corestack_pid_" + std::string(pid_str) + ".log";
    FILE *fp = fopen(coreLogFile.c_str(),"a+b");
    if (fp == NULL)
    {
        return;
    }

    fprintf(fp,"received signal %d\n",sig);
    lxPrintStack(fp);
    fclose(fp);
}

static void sigHandler(int signo)
{
    if (signo == SIGSEGV || signo == SIGBUS || signo == SIGABRT)
    {
        google::FlushLogFiles(0);
        LOG(ERROR) << "catch signal: " << signo << "\n";
        printStack(signo);
        exit(-1);
    }
    // will ignore SIGPIPE
}

static void setupSigHandler()
{
    struct sigaction sigac;
    sigemptyset(&sigac.sa_mask);
    sigac.sa_handler = sigHandler;
    sigac.sa_flags = 0;

    sigaction(SIGPIPE, &sigac, 0);
    sigaction(SIGBUS, &sigac, 0);
    sigaction(SIGSEGV, &sigac, 0);
    sigaction(SIGABRT, &sigac, 0);
    //sigaction(SIGINT, &sigac, 0);
}

int main(int argc, char** argv)
{
    if (argc < 1)
    {
        fprintf(stderr, "need configure\n");
        exit(-1);
    }

    setupSigHandler();

    const char* conf = argv[1];

    im::ImUserMessageRouter* router = new im::ImUserMessageRouter();
    ms::MsMessageCollector* pending_message_collector = new ms::MsMessageCollector();
    pending_message_collector->setCollector(new im::ImPendingMessageCollectorCore(router));
    pending_message_collector->run();

    im::MessageBroker broker;
    broker.setMessageRouter(router);
    //broker.setPendingMessageCollector(pending_message_collector);
    broker.initialize(conf);
    //broker.run();
    //broker.join();

    im::ImMessageDispatcher* imdisp = new im::ImMessageDispatcher(router, &broker);
    //imdisp->setPendingMessageCollector(pending_message_collector);
    imdisp->initialize(conf);

    io::IoAppCluster* app_io = new io::IoAppCluster(imdisp);
    app_io->setPendingMessageCollector(pending_message_collector);
    app_io->initialize(conf);
    app_io->run();

    ms::MsMessageRouterLocal* local_router = new ms::MsMessageRouterLocal(app_io);
    local_router->run();

    router->setLocalRouter(local_router);
    broker.run();

    io::IoManager manager;
    manager.setIOAppCluster(app_io);
    manager.initialize(conf);
    manager.run();
    manager.join();
}

