#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H
#include "Clock.h"
class SystemClock : public Clock
{
public:
    SystemClock()
    {
        set_clock_time(0);
    }
    virtual ~SystemClock(){}

    virtual double get_clock_time()
    {
        return m_clock_drift;
    }
    virtual void set_clock_time(double seek_time){
        m_clock_real_time = get_system_current_time();
        m_clock_drift = seek_time - m_clock_real_time;
    }
    virtual void set_clock_time(){
        long cur_time = get_system_current_time();
        m_clock_drift = m_clock_drift + m_clock_real_time-cur_time;
        m_clock_real_time=cur_time;
    }

private:    
    double m_clock_drift;
    double m_clock_real_time;
};
#endif