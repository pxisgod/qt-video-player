#ifndef CLOCK_H
#define CLOCK_H
#include "util/util.h"
#include "util/common.h"
class Clock
{
public:
    Clock()
    {
    }
    virtual ~Clock(){};
    virtual double get_clock_time()=0;
    virtual void set_clock_time(double seek_time)=0;
    virtual void set_clock_time()=0;//由于pause需要重新计算时钟
};
#endif