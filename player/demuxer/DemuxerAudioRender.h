#ifndef DEMUXER_AUDIO_RENDER_H
#define DEMUXER_AUDIO_RENDER_H
#include "Render.h"
class DemuxerAudioRender : public Render
{
public:
    explicit DemuxerAudioRender(std::shared_ptr<FrameQueue> frameQueue) : Render(frameQueue) {

                                                                          };
    virtual ~DemuxerAudioRender()
    {
    }
    virtual int init();
    virtual void render();
};

#endif