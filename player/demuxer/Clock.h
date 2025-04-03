#ifndef CLOCK_H
#define CLOCK_H
#include "util/util.h"
class Clock{
public:
    Clock(){

    }
    virtual ~Clock();
    void set_clock(long pts){
        current_time=get_system_current_time();
        current_pts=pts;
    }

    long get_clock(){
        if(current_time==-1)
            return -1;
        long now=get_system_current_time();
        return now-current_time+current_pts;
    }
private:
    long current_time=-1;
    long current_pts=-1;
};
#endif