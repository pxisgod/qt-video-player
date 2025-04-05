#ifndef V_RENDER_H
#define V_RENDER_H
#include "ThreadChain.h"
#include "util/common.h"
#include "SyncClock.h"


class VideoFrameScaler;
class VRender:public ThreadChain{
public:
    void append_frame(std::shared_ptr<AVFrame> frame);
    void set_video_frame_scaler(std::shared_ptr<VideoFrameScaler> frame_scaler);
    virtual void resize_window(){};
    virtual bool pause_condition();
    virtual bool stop_condition();
    virtual bool notify_condition();
    virtual long get_wait_time();
    virtual void deal_neg_wait_time();
    virtual int init();
    virtual void seek(long position);
    virtual void clean_func();
protected:
    std::shared_ptr<VideoFrameScaler> m_frame_scaler;
    std::shared_ptr<FrameQueue> m_frame_queue;
    int m_screen_width;
    int m_screen_height;
    std::shared_ptr<SyncClock> m_clock;
};
#endif
