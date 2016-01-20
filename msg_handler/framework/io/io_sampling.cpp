#include "io_sampling.h"
#include "lib/util.h"

#include <string.h>
#include <stdio.h>

#define CURRENT_HOUR() (utcToLocal(time(NULL)) / SECONDS_ONE_HOUR)

namespace io
{

IoSampling::IoSampling()
{
    memset(&_hour_sampling, 0, sizeof(_hour_sampling));
    _current_hour = CURRENT_HOUR();
    _current_hour_begin_second = _current_hour * SECONDS_ONE_HOUR;
}

IoSampling::~IoSampling()
{
}

void IoSampling::fixSampleSpace(time_t local)
{
    time_t hour = local / SECONDS_ONE_HOUR;
    if (expect_false(hour != _current_hour))
    {
        memset(&_hour_sampling, 0, sizeof(_hour_sampling));
        _current_hour = hour;
        _current_hour_begin_second = _current_hour * SECONDS_ONE_HOUR;
    }
}

void IoSampling::sample()
{
    time_t t = utcToLocal(time(NULL));
    fixSampleSpace(t);
    time_t s = t-_current_hour_begin_second;
    time_t m = s / 60;
    ++_hour_sampling.total;
    ++(_hour_sampling.detail._s[s].total);
    ++(_hour_sampling.detail._m[m].total);
}

void IoSampling::getSample(IoSampling::Sample& sample)
{
    sample.sec_total = 0;
    sample.min_total = 0;
    sample.hou_total = 0;

    time_t t = utcToLocal(time(NULL));
    fixSampleSpace(t);
    time_t s = t-_current_hour_begin_second;
    time_t m = s / 60;

    sample.sec_total = _hour_sampling.detail._s[s].total;
    sample.min_total = _hour_sampling.detail._m[m].total;
    sample.hou_total = _hour_sampling.total;
}

}

void printTime(const char* str)
{
    struct tm tm;
    time_t t;

    time(&t);
    localtime_r(&t, &tm);

    fprintf(stderr,"[%s]: %d-%02d-%02d %02d:%02d:%02d\n",
            str,
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec);
}
