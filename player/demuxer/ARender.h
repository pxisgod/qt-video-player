#ifndef A_RENDER_H
#define A_RENDER_H
#include "ThreadChain.h"
#include "SyncClock.h"

class AudioFrameScaler;
class ARender : public ThreadChain
{
public:
    void append_samples(std::shared_ptr<uint8_t> samples,int size,long pts);
    void set_audio_frame_scaler(std::shared_ptr<AudioFrameScaler> frame_scaler);

    virtual bool pause_condition(int work_state);
    virtual bool stop_condition();
    virtual void do_seek(long pts_time,long system_time);
    virtual void do_play(long system_time);

    std::shared_ptr<SyncClock> get_clock(){
        return m_clock;
    }

protected:
    std::shared_ptr<AudioFrameScaler> m_frame_scaler;
    std::shared_ptr<AudioSampleQueue> m_sample_queue;
    std::shared_ptr<SyncClock> m_clock;
};
#endif