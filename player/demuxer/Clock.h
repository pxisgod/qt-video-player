#ifndef CLOCK_H
#define CLOCK_H
#include "util/util.h"
#include "util/common.h"
class Clock
{
public:
    Clock()
    {}
    virtual ~Clock(){};

    virtual long get_clock(long system_time)=0;
    virtual void set_clock(long pts_time,long system_time,bool sync=true)=0;
    virtual void sync_clock(long system_time)=0; //主时钟同步
    virtual void resync_clock(long system_time)=0; //主时钟重新同步
protected:    
    long m_pts_drift; //pts时间-系统时间
    long m_last_update_time; //上次更新系统时间
};
#endif