#ifndef DEMUXER_H
#define DEMUXER_H

#include "util/common.h"
#include "ThreadChain.h"
#include "SystemClock.h"
#include <iostream>

class Track;

class Demuxer : public ThreadChain
{
public:
    using Ptr = std::shared_ptr<Demuxer>;
    explicit Demuxer(std::string &&url) : m_url(std::move(url))
    {
        m_clock = std::make_shared<SystemClock>();
    }
    Demuxer()
    {
        m_clock = std::make_shared<SystemClock>();
    }
    void set_url(std::string &&url)
    {
        m_url = std::move(url);
    }
    virtual ~Demuxer() {};
    int init_0(std::string &&url)
    {

        std::lock_guard<std::mutex> lock(m_play_mutex);
        if (m_play_state == STOPPED)
        {
            m_play_state = INITED;
            m_url = std::move(url);
            if (init() != 0)
            {
                uninit_0(); // 全部去除初始化
                return -1;
            }
            return 0;
        }

        return -1;
    }

    virtual bool pause_condition();
    virtual bool stop_condition();
    virtual long get_wait_time();
    virtual void deal_after_wait();
    virtual void deal_neg_wait_time();
    virtual int init();
    virtual void seek(long position);
    virtual int work_func();
    virtual void clean_func();
    int create_track_list();

    std::shared_ptr<AVFormatContext> get_av_format_context()
    {
        return m_av_format_context;
    }
    virtual void notify_debug()
    {
        std::cout << "Demuxer::notify_debug" << std::endl;
    }

private:
    std::string m_url;
    std::shared_ptr<AVFormatContext> m_av_format_context;
    std::weak_ptr<Track> m_track_list[2]; // 0:音频流 1：视频流
    std::map<int, std::weak_ptr<Track>> m_track_map;
    int64_t m_duration;
    std::shared_ptr<PacketQueue> m_packet_queue0; // 音频队列
    std::shared_ptr<PacketQueue> m_packet_queue1; // 视频队列
    std::shared_ptr<SystemClock> m_clock;
};

#endif // DEMUXER_H
