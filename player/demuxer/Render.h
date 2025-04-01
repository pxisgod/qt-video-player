#ifndef RENDER_H
#define RENDER_H

#include "util/common.h"
#include <memory>
#include <thread>

class Track;
class Render:public std::enable_shared_from_this<Demuxer>, EventListener<DemuxerMsg>,PlayDelegate{
    friend class Track;
    public:
    using Ptr=std::shared_ptr<Render>;
    explicit Render(std::shared_ptr<AVFrame> frameQueue){
        m_FrameQueue=frameQueue;
    }

    ~Render(){
        if(m_Thread!=nullptr){
            delete m_Thread;
            m_Thread=nullptr;
        }
    }
private:
    int init();
    void start();
    void append_frame(std::unique_ptr<AVFrame> frame);
    void notify();
    void sync();
    void stop();
    void render_thread();
    virtual void play();
    virtual void pause();
    virtual void seek(long position);
    virtual void render()=0;

private:
    std::shared_ptr<Track> m_Track; 
    std::shared_ptr<FrameQueue> m_FrameQueue;
    std::thread *m_Thread;
    std::atomic<int> m_ThreadState; //线程状态
    std::atomic<int> m_PauseState; //暂停状态
    std::recursive_mutex m_PauseMutex;
    std::condition_variable_any m_PauseCond; 
    std::mutex m_RscMutex;
};

#endif