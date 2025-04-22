#ifndef DEMUXER_H
#define DEMUXER_H

#include "util/common.h"
#include "ThreadChain.h"
#include "SystemClock.h"
#include <iostream>

enum PlayState
{
    INITED,
    PLAYING,
    PAUSE,
    STOPPED
};
class Track;
class Demuxer : public ThreadChain
{
public:
    using Ptr = std::shared_ptr<Demuxer>;
    explicit Demuxer(std::string &&url) : m_url(std::move(url))
    {
        m_clock = std::make_shared<SystemClock>();
        m_play_state = STOPPED; // 初始播放状态为STOPPED
    }
    Demuxer()
    {
        m_clock = std::make_shared<SystemClock>();
        m_play_state = STOPPED; // 初始播放状态为STOPPED
    }
    virtual ~Demuxer() {};
    void set_url(std::string &&url)
    {
        m_url = std::move(url);
    }
    std::string get_url()
    {
        return m_url;
    }
    std::shared_ptr<SystemClock> get_clock(){
        return m_clock;
    }

    int get_play_state(){
        return m_play_state;
    }

    int init_0(std::string &&url);
    void start_0();
    int seek_0(long pts_time);
    int play_0();
    int pause_0();
    void uninit_0();
    int stop_0();
    
    virtual bool pause_condition(int work_state);
    virtual int do_init(long system_time);
    virtual void do_seek(long pts_time,long system_time);
    virtual int work_func();
    virtual void clean_func();

public:
    std::shared_ptr<AVFormatContext> get_av_format_context()
    {
        return m_av_format_context;
    }

private:
    static void seek_0_l(std::list<ThreadChain::S_Ptr> & leaf_list,std::list<ThreadChain::S_Ptr> & list_copy, long pts_time);
    static void seek_1_l(std::list<ThreadChain::S_Ptr> & list_copy, long pts_time,long system_time);
    int create_track_list(long system_time);

private:
    std::string m_url;
    std::shared_ptr<AVFormatContext> m_av_format_context;
    std::weak_ptr<Track> m_track_list[2]; // 0:音频流 1：视频流
    std::map<int, std::weak_ptr<Track>> m_track_map;
    int64_t m_duration;
    std::shared_ptr<PacketQueue> m_packet_queue0; // 音频队列
    std::shared_ptr<PacketQueue> m_packet_queue1; // 视频队列
    std::shared_ptr<SystemClock> m_clock;
    std::mutex m_play_mutex;            // 播放锁
    int m_play_state;                   // 播放状态
};

#endif // DEMUXER_H
