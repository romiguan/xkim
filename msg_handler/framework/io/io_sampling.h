#ifndef __IO_SAMPLING_H__
#define __IO_SAMPLING_H__

#include <time.h>

#define SECONDS_ONE_HOUR 3600 //60*60
#define TIMEZONE_FIX_SECONDS 28800 //8*3600

namespace io
{

class IoSampling
{
    public:
        struct Sample
        {
            unsigned int sec_total;
            unsigned int min_total;
            unsigned int hou_total;
        };

    public:
        IoSampling();
        ~IoSampling();

        void sample();
        void getSample(IoSampling::Sample& sample);

    private:
        void fixSampleSpace(time_t local);

        time_t utcToLocal(time_t utc)
        {
            return utc+TIMEZONE_FIX_SECONDS;
        }

    private:
        struct Second
        {
            //设置的足够大,方便测试
            unsigned int total;
        };

        struct Minute
        {
            unsigned int total;
        };

        struct Hour
        {
            unsigned int total;
            union
            {
                Minute _m[60];
                Second _s[3600];
            } detail;
        };

    private:
        Hour _hour_sampling;
        time_t _current_hour;
        time_t _current_hour_begin_second;
};

}

void printTime(const char* str);

#endif
