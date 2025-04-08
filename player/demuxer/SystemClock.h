#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H
#include "Clock.h"
#include <iostream>
class SystemClock : public Clock
{
public:
    SystemClock()
    {
        init_clock(0);
    }
    virtual ~SystemClock(){}

    virtual double get_clock()
    {
        return m_clock_start_time;
    }
    virtual void init_clock(double seek_time){
        long cur_time = get_system_current_time();
        m_clock_start_time=cur_time-seek_time;
        m_seek_time=seek_time; //记录seek时间
    }
    virtual void set_clock(long pts)
    {
        long cur_time = get_system_current_time();
        m_seek_time=cur_time-m_clock_start_time; //记录seek时间
    }
    virtual void restart_clock(){
        long cur_time = get_system_current_time();
        m_clock_start_time=cur_time-m_seek_time; // 修改起始时间
    }

private:    
    double m_seek_time;
    double m_clock_start_time;
};
#endif