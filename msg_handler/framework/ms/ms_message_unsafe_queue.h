#ifndef __MS_MESSAGE_UNSAFE_QUEUE_H__
#define __MS_MESSAGE_UNSAFE_QUEUE_H__

namespace ms
{

class IMessage;
class MsMessageUnsafeQueue
{
    public:
        MsMessageUnsafeQueue();
        ~MsMessageUnsafeQueue();

        IMessage* getAll();
        IMessage* get();
        void put(IMessage* msg);
        int size() const { return _total_size; }

    private:
        IMessage* _head;
        IMessage* _tail;

        int _total_size;
};

}

#endif
