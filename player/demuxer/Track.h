#ifndef TRACK_H
#define TRACK_H

#include "util/common.h"
#include <memory>
#include <thread>

class Demuxer;

class Track : public std::enable_shared_from_this<Track>,EventListener<DemuxerMsg>,PlayDelegate{
    friend class Demuxer;
public:
    using Ptr=std::shared_ptr<Track>;
    explicit Track(uint32_t streamId,std::shared_ptr<Demuxer> demuxer,AVMediaType mediaType,std::shared_ptr<PacketQueue> packetQueue){
        m_StreamId=streamId;
        m_Demuxer=demuxer;
        m_PacketQueue=packetQueue;
        m_MediaType=mediaType;
    }

    ~Track(){
        if(m_Thread!=nullptr){
            delete m_Thread;
            m_Thread=nullptr;
        }
    }
private:
    int init();
    void start();
    void append_packet(std::unique_ptr<AVPacket> packet);
    void notify();
    void sync();
    void stop();
    void decoder_thread();
    void decode();
    virtual void play();
    virtual void pause();
    virtual void seek(long position);

private:
    uint32_t m_StreamId; // 轨道 ID
    std::shared_ptr<Demuxer> m_Demuxer; 
    std::shared_ptr<PacketQueue> m_PacketQueue;
    std::shared_ptr<FrameQueue> m_FrameQueue;
    AVMediaType m_MediaType;
    std::thread *m_Thread;
    std::atomic<int> m_ThreadState; //线程状态
    std::atomic<int> m_PauseState; //暂停状态
    std::recursive_mutex m_PauseMutex;
    std::condition_variable_any m_PauseCond; 
    std::mutex m_RscMutex;
    std::shared_ptr<AVCodecContext> m_AVCodecContext;
};

#endif // TRACK_H
