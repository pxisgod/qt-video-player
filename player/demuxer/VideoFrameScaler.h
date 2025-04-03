#ifndef VIDEO_FRAME_SCALER_H
#define VIDEO_FRAME_SCALER_H
#include "Scaler.h"
#include "VRender.h"
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
    static std::shared_ptr<VRender> get_thread_render(){
        return m_thread_video_render;
    }
    
    int get_video_width(){
        return m_video_width;
    }
    int get_video_height(){
        return m_video_height;
    }
    void init_sws_context(int screen_width,int screen_height);
protected:
    virtual int init();
    virtual void uninit();
    virtual int work_func();
private:
    static thread_local std::shared_ptr<VRender>  m_thread_video_render;
    std::weak_ptr<VRender>  m_video_render;
    AVPixelFormat m_src_pix_format;
    int m_video_width;
    int m_video_height;
    int m_screen_width;
    int m_screen_height;
    std::shared_ptr<SwsContext> m_sws_context;
};

#endif