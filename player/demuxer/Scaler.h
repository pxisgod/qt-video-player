#ifndef SCALER_H
#define SCALER_H

#include "util/common.h"
#include <memory>
#include <thread>
#include "EventListener.h"
#include "ThreadChain.h"
#include "SyncClock.h"
#include <iostream>

class Track;

class Scaler:public ThreadChain{
    friend class Track;
public:
    using Ptr=std::shared_ptr<Scaler>;

    explicit Scaler(std::shared_ptr<FrameQueue> frame_queue,std::shared_ptr<Track> track);

    virtual ~Scaler(){
    }
    std::shared_ptr<FrameQueue> get_frame_queue(){
        return m_frame_queue;
    }
    std::shared_ptr<SyncClock> get_clock(){
        return m_clock;
    }
    void append_frame(std::shared_ptr<AVFrame> frame);
    void render_finish();//渲染成功一帧，b_index指针往前移动一个
protected:
    virtual bool stop_condition();
    std::shared_ptr<Track> m_track; 
    std::shared_ptr<FrameQueue> m_frame_queue;
    std::shared_ptr<SyncClock> m_clock;
};

#endif