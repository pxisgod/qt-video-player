#ifndef SCALER_H
#define SCALER_H

#include "util/common.h"
#include <memory>
#include <thread>
#include "EventListener.h"
#include "ThreadChain.h"

class Track;

class Scaler:public ThreadChain, EventListener<DemuxerMsg>{
    friend class Track;
public:
    using Ptr=std::shared_ptr<Scaler>;

    explicit Scaler(std::shared_ptr<FrameQueue> frame_queue,std::shared_ptr<Track> track);

    virtual ~Scaler(){
    }
    void append_frame(std::shared_ptr<AVFrame> frame);

    std::shared_ptr<FrameQueue> get_frame_queue(){
        return m_frame_queue;
    }

    std::shared_ptr<FrameQueue> get_scale_frame_queue(){
        return m_scale_frame_queue;
    }
    AVRational get_time_base(){
        return m_time_base;
    }
    void render_finish();//渲染成功一帧，b_index指针往前移动一个


protected:
    virtual bool pause_condition();
    virtual bool stop_condition();
    virtual int init();
    virtual void seek(long position);
    virtual void clean_func();

protected:
    std::shared_ptr<Track> m_track; 
    std::shared_ptr<FrameQueue> m_frame_queue;
    std::shared_ptr<FrameQueue> m_scale_frame_queue;
    AVRational m_time_base;
};

#endif