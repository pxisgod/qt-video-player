#ifndef AUDIO_FRAME_SCALER_H
#define AUDIO_FRAME_SCALER_H
#include "Scaler.h"
#include "ARender.h"
class AudioFrameScaler : public Scaler
{
public:
    explicit AudioFrameScaler(std::shared_ptr<FrameQueue> frame_queue,std::shared_ptr<Track> track) : Scaler(frame_queue,track) {

    };
    virtual ~AudioFrameScaler()
    {
    }
private:
    static thread_local std::shared_ptr<ARender>  m_audio_render;
};

#endif