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

    explicit Scaler(std::shared_ptr<FrameQueue> frame_queue,std::shared_ptr<Track> track){
        m_frame_queue=frame_queue;
        m_track=track;
    }

    virtual ~Scaler(){
    }
protected:
    virtual bool pause_condition();
    virtual bool stop_condition();
    virtual int init();
    virtual void seek(long position);
    virtual int work_func();
    virtual void clean_func();

private:
    std::shared_ptr<Track> m_track; 
    std::shared_ptr<FrameQueue> m_frame_queue;
};

#endif