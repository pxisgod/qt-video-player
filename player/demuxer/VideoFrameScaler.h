#ifndef VIDEO_FRAME_SCALER_H
#define VIDEO_FRAME_SCALER_H
#include "Scaler.h"

class VRender;
class VideoFrameScaler : public Scaler
{
public:
    const AVPixelFormat DST_PIXEL_FORMAT = AV_PIX_FMT_YUV420P;//目标像素类型
public:
    explicit VideoFrameScaler(std::shared_ptr<FrameQueue> frame_queue,std::shared_ptr<Track> track) : Scaler(frame_queue,track) {

    };
    virtual ~VideoFrameScaler()
    {}
    static void set_thread_render(std::shared_ptr<VRender> video_render){
        m_thread_video_render=video_render;
    }
    static void remove_thread_render(){
        m_thread_video_render.reset();
    }
    static std::shared_ptr<VRender> get_thread_render(){
        return m_thread_video_render;
    }
    std::shared_ptr<FrameQueue> get_scale_frame_queue(){
        return m_scale_frame_queue;
    }
    
    int get_video_width(){
        return m_video_width;
    }
    int get_video_height(){
        return m_video_height;
    }
    void init_sws_context(int screen_width,int screen_height);
protected:
    virtual int do_init(long system_time);
    virtual void do_seek(long pts_time,long system_time);
    virtual void do_uninit();
    virtual int work_func();
    virtual void clean_func();
    virtual bool pause_condition(int work_state);
private:
    static thread_local std::shared_ptr<VRender>  m_thread_video_render;
    std::weak_ptr<VRender>  m_video_render;
    AVPixelFormat m_src_pix_format;
    int m_video_width;
    int m_video_height;
    int m_screen_width;
    int m_screen_height;
    std::shared_ptr<SwsContext> m_sws_context;
    std::shared_ptr<FrameQueue> m_scale_frame_queue;
};

#endif