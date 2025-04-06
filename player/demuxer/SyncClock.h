#ifndef SYNC_CLOCK_H
#define SYNC_CLOCK_H
#include "Clock.h"
class SyncClock : public Clock
{
public:
    SyncClock(AVRational m_time_base)
    {
        this->m_time_base = m_time_base;
        set_clock_time(0);
    }
    virtual ~SyncClock(){}
    void set_master_clock(std::shared_ptr<Clock> master_clock)
    {
        m_master_clock = master_clock;
    }

    void set_clock(long pts)
    {
        m_clock_real_time = get_system_current_time();
        m_pts = pts;
        double pts_time = ((pts * av_q2d(m_time_base)) * 1000);
        m_clock_drift = pts_time - m_clock_real_time;
    }

    double get_clock_time()
    {
        return m_clock_drift;
    }

    void set_clock_time(double seek_time)
    {
        m_clock_real_time = get_system_current_time();
        m_clock_drift = seek_time - m_clock_real_time;
        m_pts=seek_time/av_q2d(m_time_base)/1000; //计算pts
    }

    double get_sleep_time(long pts)
    {
        double delay = (((pts - m_pts) * av_q2d(m_time_base)) * 1000);
        double compute_delay=get_target_delay(delay);
        if(compute_delay<=0){
            return -1;
        }
        double current_real_time = get_system_current_time();
        double sleep_time = m_clock_real_time + compute_delay - current_real_time;
        return sleep_time;
    }

    double get_target_delay(double delay)
    {
        if (m_master_clock)
        {
            long diff = m_clock_drift - m_master_clock->get_clock_time();
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
    double m_clock_drift;
    double m_clock_real_time;
    long m_pts;
    AVRational m_time_base;
    std::shared_ptr<Clock> m_master_clock; // 主时钟
};
#endif