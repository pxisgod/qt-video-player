#ifndef V_RENDER_H
#define V_RENDER_H
#include "ThreadChain.h"
#include "util/common.h"

class VideoFrameScaler;
class VRender:public ThreadChain{
public:
    void append_frame(std::unique_ptr<AVFrame> frame);
    void set_video_frame_scaler(std::shared_ptr<VideoFrameScaler> frame_scaler){
        m_frame_scaler=frame_scaler;
        m_frame_queue=frame_scaler->get_scale_frame_queue();
    }
    virtual void resize_window()=0;
    virtual bool pause_condition();
    virtual bool stop_condition();
    virtual int init();
    virtual void seek(long position);
    virtual void clean_func();
protected:
    std::shared_ptr<VideoFrameScaler> m_frame_scaler;
    std::shared_ptr<FrameQueue> m_frame_queue;
    int screen_width;
    int screen_height;
};
#endif;
