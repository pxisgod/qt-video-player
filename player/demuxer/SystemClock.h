#ifndef SYSTEMCLOCK_H
#define SYSTEMCLOCK_H
#include "Clock.h"
#include <iostream>
class SystemClock : public Clock
{
public:
    SystemClock()
    {
    }
    virtual ~SystemClock(){}

    long get_clock(long system_time)
    {
        std::lock_guard<std::mutex> lock(m_clock_mutex);
        return m_pts_drift + system_time;
    }

    void set_clock(long pts_time,long system_time,bool sync=true)
    {
        std::lock_guard<std::mutex> lock(m_clock_mutex);
        m_pts_drift = pts_time - system_time; // 记录pts时间和系统时间的差值
        m_last_update_time = system_time; // 记录上次更新时间
    }

    virtual void sync_clock(long system_time) //主时钟同步
    {
        std::lock_guard<std::mutex> lock(m_clock_mutex);
        m_last_update_time = system_time; // 更新上次更新时间
    }
    virtual void resync_clock(long system_time){ //主时钟重新同步
        std::lock_guard<std::mutex> lock(m_clock_mutex);
        m_pts_drift = m_pts_drift + m_last_update_time - system_time ; // 更新pts时间和系统时间的差值
        m_last_update_time = system_time; // 更新上次更新时间
    }
private:
    std::mutex m_clock_mutex;
};
#endif