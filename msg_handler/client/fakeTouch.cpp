#include "proto/im.pb.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "error\n");
        exit(-1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        fprintf(stderr, "create socket error, abort\n");
        abort();
    }

    struct sockaddr_in sin;
    ::memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(argv[2]));
    sin.sin_addr.s_addr = inet_addr(argv[1]);

    int ret = connect(fd, (struct sockaddr*)&sin, sizeof(sin));
    if (ret == -1)
    {
        fprintf(stderr, "connect error\n");
        exit(-1);
    }

    while (true)
    {
        im::ImPack pack;
        //pack.set_type(im::IM_TOUCH);
        im::ImPackContent* content = pack.mutable_content();
        content->mutable_touch()->set_uid(atoi(argv[3]));
        int size = pack.ByteSize();
        char buf[1024];
        *((int*)buf) = htonl(size+4);
        memcpy(buf+4, "mimi", 4);
        pack.SerializeToArray(buf+8, size);

        int mmret = write(fd, buf, size+8);
        (void)mmret;

        memset(buf, 0, 1024);
        ret = read(fd, buf, 1024);
        im::ImPack rpack;
        rpack.ParseFromArray(buf+8, ret-8);
        fprintf(stderr, "%d:%x:%d\n", ret,
                *((int*)(buf+4)),
                rpack.content().touch_rep().uid());

        sleep(1);
    }

    close(fd);
    exit(0);
}
