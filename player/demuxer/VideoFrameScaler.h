#ifndef VIDEO_FRAME_SCALER_H
#define VIDEO_FRAME_SCALER_H
#include "Scaler.h"
#include "VRender.h"
class VideoFrameScaler : public Scaler
{
public:
    explicit VideoFrameScaler(std::shared_ptr<FrameQueue> frame_queue,std::shared_ptr<Track> track) : Scaler(frame_queue,track) {

    };
    virtual ~VideoFrameScaler()
    {
    }
private:
    static thread_local<VRendor>  m_video_render;
};

#endif