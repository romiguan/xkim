#include "proto/im.pb.h"

#include "framework/lib/thread.h"
#include "framework/lib/posix_thread_factory.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <string>

int g_seq = 1;
int g_me;

void setTouch(int fd, int uid);

class Touch : public util::Runnable
{
    public:
        Touch(int fd, int uid):
            _fd(fd),
            _uid(uid)
        {
        }
        virtual ~Touch() {}

        virtual int run();

        void setUid(int uid) { _uid = uid; }
        void setFd(int fd) { _fd = fd; }

    private:
        int _fd;
        int _uid;
};

int Touch::run()
{
    while (true)
    {
        setTouch(_fd, _uid);
        sleep(5);
    }
    return 0;
}

void setTouch(int fd, int uid)
{
    char buf[1024];

    im::ImPack pack;
    pack.set_type(im::IM_TOUCH);
    pack.set_ts(time(NULL)-1420041600);
    im::ImPackContent* content = pack.mutable_content();
    content->mutable_touch()->set_uid(uid);
    int size = pack.ByteSize();
    *((int*)buf) = htonl(size+4);
    memcpy(buf+4, "mimi", 4);
    pack.SerializeToArray(buf+8, size);

    int ret = write(fd, buf, size+8);
    (void)ret;
}

void startTouch(int fd, int uid)
{
    util::PosixThreadFactory ptf;
    ptf.setPolicy(util::PosixThreadFactory::OTHER);
    ptf.setStackSize(10);
    ptf.setDetached(true);

    boost::shared_ptr<util::Runnable> core(new Touch(fd, uid));
    boost::shared_ptr<util::Thread> thread = ptf.newThread(core);
    thread->run();
}

void auth(int fd, int uid)
{
    char buf[1024];

    im::ImPack pack;
    pack.set_type(im::IM_AUTH);
    pack.set_ts(time(NULL)-1420041600);
    im::ImPackContent* content = pack.mutable_content();
    im::ImAuthRequest* msg = content->mutable_auth_req();
    msg->set_uid(uid);
    msg->set_token("token");

    int size = pack.ByteSize();
    *((int*)buf) = htonl(size+4);
    memcpy(buf+4, "mimi", 4);
    pack.SerializeToArray(buf+8, size);

    int ret = write(fd, buf, size+8);

    int frame_size = 0;
    char* p = buf;
    int pos = 0;
    int want_read = 1024;
    while (true)
    {
        ret = read(fd, p+pos, want_read-pos);
        if (ret == 0)
        {
            fprintf(stderr, "quit\n");
            exit(0);
        }
        else if (ret == -1)
        {
            if (errno == EINTR)
                continue;
            else
            {
                fprintf(stderr, "error, %s: %d\n", strerror(errno), pos);
                exit(-1);
            }
        }

        pos += ret;
        if (pos >= (int)sizeof(int))
        {
            frame_size = ntohl(*((int*)p));
            if (frame_size+(int)sizeof(int) <= pos)
            {
                im::ImPack rpack;
                rpack.ParseFromArray(p+8, frame_size-4);
                int type = rpack.type();
                if (type != im::IM_AUTH_RESPONSE)
                    exit(-1);

                assert(rpack.content().has_auth_rep());
                bool s = rpack.content().auth_rep().auth_success();
                if (s == false)
                {
                    fprintf(stderr, "auth fail\n");
                }
                else
                {
                    fprintf(stderr, "auth ok\n");
                    return;
                }
            }
        }
    }
}

void printMsg(im::ImPack& pack)
{
    const im::ImMsg& msg = pack.content().msg();
    int from_uid = msg.uid();
    int seq = msg.seq();
    int target_uid_count = msg.target_uid_size();
    if (target_uid_count != 1)
    {
        for (int i = 0; i < target_uid_count; ++i)
            fprintf(stderr, "%d\n", msg.target_uid(i));
        fprintf(stderr, "target more, abort\n");
        abort();
    }
    int target_uid = msg.target_uid(0);
    if (target_uid != g_me)
    {
        fprintf(stderr, "target uid not me, abort\n");
        abort();
    }

    unsigned long long id;
    memcpy(&id, msg.msg_id().c_str(), msg.msg_id().size());

    std::string text;
    if (msg.has_text_content())
        text = msg.text_content();

    fprintf(stderr, "from: %d, to: %d, seq: %d, id: %llu, message: %s\n",
            from_uid,
            target_uid,
            seq,
            id,
            text.c_str());
}

void printMsgAck(im::ImPack& pack)
{
    const im::ImMsgAck& msg_ack = pack.content().msg_ack();
    assert(msg_ack.has_msg_id());

    unsigned long long id;
    memcpy(&id, msg_ack.msg_id().c_str(), msg_ack.msg_id().size());
    fprintf(stderr, "msg ack [%d: %llu]\n", pack.ts(), id);
}

void setMsg(int fd, int uid, const char* message, int* target_uid, int target_count)
{
    char buf[1024];

    im::ImPack pack;
    pack.set_type(im::IM_MSG);
    pack.set_ts(time(NULL)-1420041600);
    im::ImPackContent* content = pack.mutable_content();
    im::ImMsg* msg = content->mutable_msg();
    msg->set_uid(uid);
    msg->set_seq(g_seq++);
    for (int i = 0; i < target_count; ++i)
        msg->add_target_uid(target_uid[i]);
    msg->set_type(im::IM_REQUEST);
    msg->set_text_content(message);

    int size = pack.ByteSize();
    *((int*)buf) = htonl(size+4);
    memcpy(buf+4, "mimi", 4);
    pack.SerializeToArray(buf+8, size);

    int ret = write(fd, buf, size+8);
    (void)ret;
}

int main(int argc, char** argv)
{
    int target_uid[1024];
    int uid = atoi(argv[3]);
    g_me = uid;
    const char* message = argv[4];
    int target_count = argc-5;
    for (int i = 0; i < target_count; ++i)
        target_uid[i] = atoi(argv[5+i]);

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

    auth(fd, uid);
    startTouch(fd, uid);

    char buf[1024];
    int frame_size = 0;
    char* p = buf;
    int want_read = 1024;
    int pos = 0;
    while (true)
    {
        if (argc > 5)
            setMsg(fd, uid, message, target_uid, target_count);

        if (pos < 1024)
        {
            ret = read(fd, p+pos, want_read-pos);
            if (ret == 0)
            {
                fprintf(stderr, "quit\n");
                exit(0);
            }
            else if (ret == -1)
            {
                if (errno == EINTR)
                    continue;
                else
                {
                    fprintf(stderr, "error, %s: %d\n", strerror(errno), pos);
                    exit(-1);
                }
            }

            pos += ret;
        }

        while (pos > (int)sizeof(int))
        {
            frame_size = ntohl(*((int*)p));
            if (frame_size+(int)sizeof(int) <= pos)
            {
                im::ImPack rpack;
                rpack.ParseFromArray(p+8, frame_size-4);
                int type = rpack.type();

                switch (type)
                {
                    case im::IM_TOUCH_ACK:
                        fprintf(stderr, "touch ack [%d: %d]\n", rpack.ts(), rpack.content().touch_rep().uid());
                        break;
                    case im::IM_MSG:
                        printMsg(rpack);
                        break;
                    case im::IM_MSG_ACK:
                        printMsgAck(rpack);
                        break;
                    default:
                        fprintf(stderr, "%d: error\n", type);
                        break;
                }

                memcpy(p, p+frame_size+sizeof(int), pos-frame_size-sizeof(int));
                pos = pos-frame_size-sizeof(int);
            }
            else
            {
                break;
            }
        }

        sleep(1);
    }

    close(fd);
    exit(0);
}
