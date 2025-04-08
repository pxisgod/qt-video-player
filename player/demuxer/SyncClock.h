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
        init_clock(0);
    }
    virtual ~SyncClock(){}
    void set_master_clock(std::shared_ptr<Clock> master_clock)
    {
        m_master_clock = master_clock;
    }
    std::shared_ptr<Clock> get_master_clock(){
        return m_master_clock;
    }

    double get_clock()
    {
        return m_clock_start_time;
    }

    void init_clock(double seek_time)
    {
        long cur_time = get_system_current_time();
        m_clock_start_time=cur_time-seek_time;
        m_seek_time=seek_time; //记录seek时间
        m_pts=seek_time/av_q2d(m_time_base)/1000; //计算pts
    }

    void set_clock(long pts)
    {
        long cur_time = get_system_current_time();
        m_pts = pts;
        m_seek_time = ((pts * av_q2d(m_time_base)) * 1000);
        m_clock_start_time = cur_time-m_seek_time;
        if(m_master_clock){
            m_master_clock->set_clock(pts);
        }
    }
    
    virtual void restart_clock(){
        long cur_time = get_system_current_time();
        m_clock_start_time=cur_time-m_seek_time; // 修改起始时间
    }

    double get_sleep_time(long pts)
    {
        double delay = (((pts - m_pts) * av_q2d(m_time_base)) * 1000);
        double compute_delay=get_target_delay(delay);
        if(compute_delay<=0){
            return -1;
        }
        double cur_time = get_system_current_time();
        double sleep_time = m_clock_start_time +m_seek_time + compute_delay - cur_time;
        return sleep_time;
    }

    double get_target_delay(double delay)
    {
        if (m_master_clock)
        {
            long diff = m_master_clock->get_clock()-get_clock();
            double threshold = std::max(40.0, std::min(100.0, delay));
            if (diff < -threshold)
            {
                return 0;
            }
            else if (diff >= threshold && delay > 100.0)
            {
                return diff + delay;
            }
            else if (diff >= threshold)
            {
                return delay * 2;
            }
        }
        return delay;
    }
private:
    double m_seek_time;
    double m_clock_start_time;
    long m_pts;
    AVRational m_time_base;
    std::shared_ptr<Clock> m_master_clock; // 主时钟
};
#endif