#ifndef DEMUXER_H
#define DEMUXER_H

#include "util/common.h"
#include "ThreadChain.h"

class Track;

class Demuxer : public ThreadChain, EventListener<DemuxerMsg>
{
public:
    using Ptr = std::shared_ptr<Demuxer>;
    explicit Demuxer(std::string &&url) : m_url(std::move(url))
    {
    }

    virtual ~Demuxer(){}

    virtual bool pause_condition();
    virtual int init();
    virtual void seek(long position);
    virtual int work_func();
    virtual void clean_func();
    int create_track_list();
    
    std::shared_ptr<AVFormatContext> get_av_format_context(){
        return m_av_format_context;
    }
private:
    std::string m_url;
    std::shared_ptr<AVFormatContext> m_av_format_context;
    std::weak_ptr<Track> m_track_list[2]; // 0:音频流 1：视频流
    std::map<int, std::weak_ptr<Track>> m_track_map;
    int64_t m_duration;
    std::shared_ptr<PacketQueue> m_packet_queue0; //音频队列
    std::shared_ptr<PacketQueue> m_packet_queue1;  //视频队列
};

#endif // DEMUXER_H
