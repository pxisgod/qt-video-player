#ifndef TRACK_H
#define TRACK_H

#include "util/common.h"
#include <memory>
#include <thread>
#include "EventListener.h"
#include "PlayDelegate.h"
#include "Render.h"

class Demuxer;

class Track : public ThreadChain,EventListener<DemuxerMsg>{
    friend class Demuxer;
public:
    using Ptr=std::shared_ptr<Track>;
    explicit Track(uint32_t streamId,std::shared_ptr<Demuxer> demuxer,AVMediaType mediaType,std::shared_ptr<PacketQueue> packetQueue){
        m_StreamId=streamId;
        m_Demuxer=demuxer;
        m_PacketQueue=packetQueue;
        m_MediaType=mediaType;
    }

    virtual ~Track(){
    }
private:
    virtual bool pause_condition();
    virtual bool stop_condition();
    virtual int init();
    virtual void seek(long position);
    virtual int work_func();
    virtual void clean_func();
    void append_packet(std::unique_ptr<AVPacket> packet);

private:
    uint32_t m_StreamId; // 轨道 ID
    AVMediaType m_MediaType;
    std::shared_ptr<Demuxer> m_Demuxer; 
    std::weak_ptr<Render> m_Render;
    std::shared_ptr<PacketQueue> m_PacketQueue;
    std::shared_ptr<FrameQueue> m_FrameQueue;
    std::shared_ptr<AVCodecContext> m_AVCodecContext;
};

#endif // TRACK_H
