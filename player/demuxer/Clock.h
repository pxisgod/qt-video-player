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

    virtual double get_clock()=0;
    virtual void init_clock(double seek_time)=0;
    virtual void set_clock(long pts)=0;
    virtual void restart_clock()=0;
};
#endif