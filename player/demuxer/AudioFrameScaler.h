#ifndef AUDIO_FRAME_SCALER_H
#define AUDIO_FRAME_SCALER_H
#include "Scaler.h"

// 音频编码采样率
static const int AUDIO_DST_SAMPLE_RATE = 44100;
// 音频编码声道格式
static const uint64_t AUDIO_DST_CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
// 音频样本格式
static const AVSampleFormat AUDIO_DST_SAMPLE_FORMAT = AV_SAMPLE_FMT_S16;
// 音频样本大小
static const int AUDIO_SAMPLE_SIZE = 2;
// 音频一帧采样数
static const int AUDIO_NB_SAMPLES = 2048;
// 音频编码通道数
static const int AUDIO_DST_CHANNEL_COUNTS = 2;

class ARender;
class AudioFrameScaler : public Scaler
{

public:
    explicit AudioFrameScaler(std::shared_ptr<FrameQueue> frame_queue, std::shared_ptr<Track> track) : Scaler(frame_queue, track) {

                                                                                                       };
    virtual ~AudioFrameScaler()
    {
    }

    static void set_thread_render(std::shared_ptr<ARender> audio_render)
    {
        m_thread_audio_render = audio_render;
    }
    static void remove_thread_render()
    {
        m_thread_audio_render.reset();
    }
    static std::shared_ptr<ARender> get_thread_render()
    {
        return m_thread_audio_render;
    }
    std::shared_ptr<AudioSampleQueue> get_scale_sample_queue()
    {
        return m_scale_sample_queue;
    }

protected:
    virtual int do_init(long system_time);
    virtual void do_seek(long pts_time, long system_time);
    virtual void do_uninit();
    virtual int work_func();
    virtual void clean_func();
    virtual bool pause_condition(int work_state);

private:
    static thread_local std::shared_ptr<ARender> m_thread_audio_render;
    std::weak_ptr<ARender> m_audio_render;
    std::shared_ptr<AudioSampleQueue> m_scale_sample_queue;
    int m_scale_buffer_size = 0;
    std::shared_ptr<uint8_t> m_scale_buffer;
    int m_src_sample_rate;
    uint64_t m_src_channel_layout;
    AVSampleFormat m_src_sample_format;
    std::shared_ptr<SwrContext> m_swr_context;
};

#endif