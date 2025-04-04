#ifndef TRACK_H
#define TRACK_H

#include "util/common.h"
#include <memory>
#include <thread>
#include "EventListener.h"
#include "ThreadChain.h"

class Demuxer;

class Scaler;

class Track : public ThreadChain,EventListener<DemuxerMsg>{
    friend class Demuxer;
public:
    using Ptr=std::shared_ptr<Track>;
    explicit Track(uint32_t stream_id,std::shared_ptr<Demuxer> demuxer,AVMediaType media_type,std::shared_ptr<PacketQueue> packet_queue);

    virtual ~Track(){
    }
    std::shared_ptr<AVCodecContext> get_av_codec_context(){
        return m_av_codec_context;
    }
    AVRational get_time_base(){
        return m_time_base;
    }
protected:
    virtual bool pause_condition();
    virtual bool stop_condition();
    virtual int init();
    virtual void seek(long position);
    virtual int work_func();
    virtual void clean_func();
private:
    void append_packet(std::shared_ptr<AVPacket> packet);
    int create_scaler();


private:
    uint32_t m_stream_id; // 轨道 ID
    std::shared_ptr<Demuxer> m_demuxer; 
    AVMediaType m_media_type;
    std::shared_ptr<PacketQueue> m_packet_queue;
    std::weak_ptr<Scaler> m_scaler;
    std::shared_ptr<FrameQueue> m_frame_queue;
    std::shared_ptr<AVCodecContext> m_av_codec_context;
    AVRational m_time_base;

};

#endif 
