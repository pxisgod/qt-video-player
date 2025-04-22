#ifndef VIDEOSYNCCLOCK_H
#define VIDEOSYNCCLOCK_H
#include "SyncClock.h"
#include <iostream>
class VideoSyncClock : public SyncClock
{
public:
    VideoSyncClock(AVRational m_time_base) : SyncClock(m_time_base)
    {
    }
    virtual ~VideoSyncClock(){}
    virtual long get_real_delay(long clock_diff,long delay)
    {
        long threshold = std::max(40l, std::min(100l, delay));
        if (clock_diff < -threshold)
        {
            return 0;
        }
        else if (clock_diff >= threshold && delay > 100)
        {
            return clock_diff + delay;
        }
        else if (clock_diff >= threshold)
        {
            return delay * 2;
        }
        return delay;
    }
};
#endif