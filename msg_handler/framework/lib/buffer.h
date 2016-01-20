#ifndef __BUFFER_H__
#define __BUFFER_H__

namespace buffer
{

class SimpleBuffer
{
    public:
        explicit SimpleBuffer(int capacity);

        ~SimpleBuffer();

        char* getBuffer() { return _buffer; }

    private:
        char* _buffer;
};

class ReadBuffer
{
    public:
        explicit ReadBuffer(int capacity):
            _buffer(capacity),
            _mem(_buffer.getBuffer()),
            _capacity(capacity),
            _len(0)
        {
        }

        ~ReadBuffer() {}

    private:
        SimpleBuffer _buffer;
        char* _mem;
        int _capacity;
        int _len;
};

class WriteBuffer
{
    public:
        explicit WriteBuffer(int capacity):
            _buffer(capacity),
            _mem(_buffer.getBuffer()),
            _capacity(capacity),
            _len(0),
            _pos(0)
        {
        }

        void writeToBuffer(const char* w, int len);

        ~WriteBuffer() {}

    private:
        SimpleBuffer _buffer;
        char* _mem;
        int _capacity;
        int _len;
        int _pos;
};

}

#endif
