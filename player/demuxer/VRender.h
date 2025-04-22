#ifndef V_RENDER_H
#define V_RENDER_H
#include "ThreadChain.h"
#include "util/common.h"
#include "SyncClock.h"
#include <iostream>


class VideoFrameScaler;
class VRender:public ThreadChain{
public:
    void append_frame(std::shared_ptr<AVFrame> frame);
    void set_video_frame_scaler(std::shared_ptr<VideoFrameScaler> frame_scaler);
    virtual void resize_window(){};
    virtual bool pause_condition(int work_state);
    virtual bool stop_condition();
    virtual int do_init(long system_time);
    virtual void do_seek(long pts_time,long system_time);
    virtual void do_play(long system_time);
    std::shared_ptr<SyncClock> get_clock(){
        return m_clock;
    }
protected:
    std::shared_ptr<VideoFrameScaler> m_frame_scaler;
    std::shared_ptr<FrameQueue> m_frame_queue;
    int m_screen_width;
    int m_screen_height;
    std::shared_ptr<SyncClock> m_clock;
};
#endif
