#include "buffer.h"

#include <stdlib.h>

#include <new>

namespace buffer
{

SimpleBuffer::SimpleBuffer(int capacity)
{
    if (capacity <= 0)
        abort();

    _buffer = new (std::nothrow) char [capacity];
    if (_buffer == 0)
        abort();
}

SimpleBuffer::~SimpleBuffer()
{
    if (_buffer)
        delete [] _buffer;
}

}

