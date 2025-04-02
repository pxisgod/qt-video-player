#ifndef DEMUXER_VIDEO_RENDER_H
#define DEMUXER_VIDEO_RENDER_H
#include "Render.h"
class DemuxerVideoRender : public Render
{
public:
    explicit DemuxerVideoRender(std::shared_ptr<FrameQueue> frameQueue) : Render(frameQueue) {

                                                                          };
    virtual ~DemuxerVideoRender()
    {
    }
    virtual int init();
    virtual int init_l()=0;
    virtual void render();
};

#endif