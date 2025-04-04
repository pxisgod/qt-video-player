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
        return m_clock_drift + get_system_current_time();
    }
    virtual void set_clock_time(double seek_time){
        m_clock_real_time = get_system_current_time();
        m_clock_drift = seek_time - m_clock_real_time;
    }

private:    
    double m_clock_drift;
    double m_clock_real_time;
};
#endif