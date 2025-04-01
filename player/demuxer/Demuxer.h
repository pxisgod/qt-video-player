#ifndef DEMUXER_H
#define DEMUXER_H

#include <memory>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "util/common.h"
#include "EventListener.h"
#include "PlayDelegate.h"

class Track;

class Demuxer : public std::enable_shared_from_this<Demuxer>, EventListener<DemuxerMsg>,PlayDelegate
{
public:
    using Ptr = std::shared_ptr<Demuxer>;
    using AvFormatContextPtr = std::shared_ptr<AVFormatContext>;
    explicit Demuxer(std::string &&url) : m_Url(std::move(url))
    {
    }

    virtual ~Demuxer()
    {
        if (m_Thread != nullptr)
        {
            delete m_Thread;
            m_Thread = nullptr;
        }
    }

    int start();
    int createTrackList();
    void notify();
    void sync();
    void stop();
    void demuxer_thread();
    virtual void play();
    virtual void pause();
    virtual void seek(long position);
    
    

    AvFormatContextPtr getAVFormatContext(){
        return m_AVFormatContext;
    }

    int get_thread_state(){
        return m_ThreadState.load(std::memory_order_seq_cst);
    }

private:
    AvFormatContextPtr m_AVFormatContext;
    std::string m_Url;
    std::weak_ptr<Track> m_TrackList[2]; // 0:音频流 1：视频流
    std::map<int, std::weak_ptr<Track>> m_TrackMap;
    std::thread *m_Thread;
    int64_t m_Duration;
    std::atomic<int> m_ThreadState; // 线程状态
    std::atomic<int> m_PauseState;  // 暂停状态
    std::recursive_mutex m_PauseMutex; //暂停条件锁
    std::condition_variable_any m_PauseCond; //暂停条件
    std::mutex m_RscMutex; //资源锁
    std::shared_ptr<PacketQueue> m_PacketQueue0; //音频队列
    std::shared_ptr<PacketQueue> m_PacketQueue1;  //视频队列
};

#endif // DEMUXER_H
