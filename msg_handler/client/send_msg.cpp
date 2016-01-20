#include "proto/im.pb.h"
#include "thrift/gen/Message.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <thrift/transport/TSocket.h>  
#include <thrift/transport/TBufferTransports.h>  
#include <thrift/protocol/TBinaryProtocol.h>  

using namespace apache::thrift;  
using namespace apache::thrift::protocol;  
using namespace apache::thrift::transport;

int main(int argc, char** argv)
{
    if (argc < 6)
    {
        fprintf(stderr, "error\n");
        exit(-1);
    }

    ms::MessageRequest request;
    ms::MessageResponse response;

    boost::shared_ptr<TSocket> socket(new TSocket(argv[1], atoi(argv[2])));  
    boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));  
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));  
    transport->open();

    ms::MessageClient client(protocol);

    int seq = 0;
    while (true)
    {
        im::ImPack pack;
        pack.set_type(im::IM_MSG);
        im::ImPackContent* content = pack.mutable_content();
        im::ImMsg* msg_pack = content->mutable_msg();
        msg_pack->set_uid(atoi(argv[4]));
        msg_pack->set_seq(++seq);

        int i = 5;
        while (i < argc)
        {
            msg_pack->add_target_uid(atoi(argv[i]));
            ++i;
        }

        msg_pack->set_type(im::IM_REQUEST);
        msg_pack->set_text_content(argv[3], strlen(argv[3]));

        pack.SerializeToString(&request.msg);
        request.__isset.msg = true;

        try
        {
            client.dispatch(response, request);
        }
        catch (...)
        {
            fprintf(stderr, "quit\n");
            exit(0);
        }

        sleep(1);
    }

    transport->close();
    exit(0);
}
