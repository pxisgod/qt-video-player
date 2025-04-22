#ifndef SYNC_CLOCK_H
#define SYNC_CLOCK_H
#include "Clock.h"
#include <iostream>
class SyncClock : public Clock
{
public:
    SyncClock(AVRational m_time_base)
    {
        this->m_time_base = m_time_base;
    }
    virtual ~SyncClock(){}
    void set_master_clock(std::shared_ptr<Clock> master_clock)
    {
        m_master_clock = master_clock;
    }
    std::shared_ptr<Clock> get_master_clock(){
        return m_master_clock;
    }

    void set_clock_by_pts(long pts,long system_time)
    {
        m_pts = pts;
        long pts_time = ((pts * av_q2d(m_time_base)) * 1000);
        set_clock(pts_time, system_time,false);
    }

    long get_clock(long system_time)
    {
        std::lock_guard<std::recursive_mutex> lock(m_clock_mutex);
        return m_pts_drift + system_time;
    }

    void set_clock(long pts_time,long system_time,bool sync=true)
    {
        std::lock_guard<std::recursive_mutex> lock(m_clock_mutex);
        m_pts = pts_time/av_q2d(m_time_base)/1000;
        m_pts_drift = pts_time - system_time; // 记录pts时间和系统时间的差值
        m_last_update_time = system_time; // 记录上次更新时间
        if (m_master_clock)
        {
            if(!sync)
            {
                m_master_clock->sync_clock( system_time); // 设置主时钟
            }
            else
            {
                m_master_clock->set_clock(pts_time, system_time,true); // 设置主时钟
            }
        }
    }

    void sync_clock(long system_time){} //主时钟同步
    
    void resync_clock(long system_time){ //主时钟重新同步
        std::lock_guard<std::recursive_mutex> lock(m_clock_mutex);
        m_pts_drift = m_pts_drift + m_last_update_time - system_time ; // 更新pts时间和系统时间的差值
        m_last_update_time = system_time; // 更新上次更新时间
        if (m_master_clock)
        {
            m_master_clock->resync_clock(system_time); // 重新同步主时钟
        }
    }
    
    virtual long get_target_delay(long pts,long system_time)
    {
        std::lock_guard<std::recursive_mutex> lock(m_clock_mutex);
        long delay = (((pts-m_pts) * av_q2d(m_time_base)) * 1000);
        if (m_master_clock)
        {
            long clock_diff = get_clock(system_time)-m_master_clock->get_clock(system_time);
            return get_real_delay(clock_diff,delay);
        }
        return delay;
    }

    virtual long get_real_delay(long clock_diff,long delay)=0;

    AVRational get_time_base()
    {
        return m_time_base;
    }
   
protected:
    std::recursive_mutex m_clock_mutex;
    long m_pts;
    AVRational m_time_base;
    std::shared_ptr<Clock> m_master_clock; // 主时钟
};
#endif