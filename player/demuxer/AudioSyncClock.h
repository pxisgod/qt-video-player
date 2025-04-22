#ifndef AUDIOSYNCCLOCK_H
#define AUDIOSYNCCLOCK_H
#include "SyncClock.h"
#include <iostream>
class AudioSyncClock : public SyncClock
{
public:
    AudioSyncClock(AVRational m_time_base) : SyncClock(m_time_base)
    {
    }
    virtual ~AudioSyncClock(){}

    virtual long get_real_delay(long clock_diff,long delay)
    {
        long threshold = std::max(10l, std::min(20l, delay));
        if (clock_diff < -threshold || clock_diff > threshold)
        {
            return clock_diff;
        }
        return 0;
    }
};
#endif