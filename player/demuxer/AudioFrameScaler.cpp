#include "AudioFrameScaler.h"
#include "ARender.h"
#include "Track.h" 
thread_local std::shared_ptr<ARender> AudioFrameScaler::m_thread_audio_render;

int AudioFrameScaler::do_init(long system_time)
{
    m_scale_sample_queue = std::make_shared<AudioSampleQueue>(AUDIO_DST_SAMPLE_RATE,AUDIO_SAMPLE_SIZE,AUDIO_DST_CHANNEL_COUNTS,AUDIO_NB_SAMPLES,m_clock->get_time_base()); // 创建scale后的frame_queue
    SwrContext *swr_context= swr_alloc();
    m_swr_context = std::shared_ptr<SwrContext>(swr_context, [](SwrContext *swr_context_ptr)
                                                 {
        if(swr_context_ptr != nullptr) {
            swr_free(&swr_context_ptr);
        } });
    m_src_sample_rate=m_track->get_av_codec_context()->sample_rate;
    m_src_channel_layout=m_track->get_av_codec_context()->channel_layout;
    m_src_sample_format=m_track->get_av_codec_context()->sample_fmt;
    if(m_track->get_codecpar()->frame_size>0)
    {
        int src_nb_samples= m_track->get_codecpar()->frame_size;
        int dst_nb_samples = (int)av_rescale_rnd(src_nb_samples, AUDIO_DST_SAMPLE_RATE, m_src_sample_rate, AV_ROUND_UP);
        m_scale_buffer_size = av_samples_get_buffer_size(NULL, AUDIO_DST_CHANNEL_COUNTS,dst_nb_samples,AUDIO_DST_SAMPLE_FORMAT, 1);
        uint8_t *scale_buffer = (uint8_t *) av_malloc(m_scale_buffer_size);
        m_scale_buffer = std::shared_ptr<uint8_t>(scale_buffer, [](uint8_t *ptr)
                                                 {
            if(ptr != nullptr)
                av_free(ptr);
        });
    }

    av_opt_set_int(swr_context, "in_channel_layout", m_src_channel_layout, 0);
    av_opt_set_int(swr_context, "out_channel_layout", AUDIO_DST_CHANNEL_LAYOUT, 0);
    av_opt_set_int(swr_context, "in_sample_rate", m_src_sample_rate, 0);
    av_opt_set_int(swr_context, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);
    av_opt_set_sample_fmt(swr_context, "in_sample_fmt", m_src_sample_format, 0);
    av_opt_set_sample_fmt(swr_context, "out_sample_fmt", AUDIO_DST_SAMPLE_FORMAT,  0);

    if(swr_init(swr_context)<0){
        return -1;
    }

    if (m_thread_audio_render != nullptr)
    {
        std::shared_ptr<AudioFrameScaler> scaler = std::static_pointer_cast<AudioFrameScaler>(shared_from_this());
        m_thread_audio_render->set_audio_frame_scaler(scaler);
        m_audio_render = m_thread_audio_render;
        add_thread(m_thread_audio_render);
        return m_thread_audio_render->init(system_time);
    }else
    {
        return -1;
    }
}

void AudioFrameScaler::do_seek(long pts_time,long system_time){
    m_scale_sample_queue->clear();
}

void AudioFrameScaler::do_uninit()
{
    m_thread_audio_render = nullptr; // 释放强指针
}

int AudioFrameScaler::work_func()
{
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    if (m_frame_queue->is_empty())
    {
        return 0;
    }
    std::shared_ptr<AVFrame> frame = m_frame_queue->read_frame();

    int dst_nb_samples = (int)av_rescale_rnd(frame->nb_samples, AUDIO_DST_SAMPLE_RATE, m_src_sample_rate, AV_ROUND_UP);
    int scale_buffer_size = av_samples_get_buffer_size(NULL, AUDIO_DST_CHANNEL_COUNTS,dst_nb_samples, AUDIO_DST_SAMPLE_FORMAT, 1);
    uint8_t* scale_buffer=m_scale_buffer.get();
    if (scale_buffer_size > m_scale_buffer_size)
    {
        scale_buffer=(uint8_t*)av_malloc(scale_buffer_size);
        m_scale_buffer = std::shared_ptr<uint8_t>(scale_buffer, [](uint8_t *ptr)
                                                 {
            if(ptr != nullptr)
                av_free(ptr);
        });
        m_scale_buffer_size = scale_buffer_size;
    }

    int ret = swr_convert(m_swr_context.get(), &scale_buffer, scale_buffer_size / AUDIO_DST_CHANNEL_COUNTS, 
                            (const uint8_t **) frame->data, frame->nb_samples);
    if (ret < 0)
    {
        char err_buf[AV_ERROR_MAX_STRING_SIZE];
        av_make_error_string(err_buf, AV_ERROR_MAX_STRING_SIZE, ret);
        qDebug() << "swr_convert failed: " << err_buf;
        return -1;
    }

    if (auto render = m_audio_render.lock())
    {
        long pts;
        if (frame->pkt_dts != AV_NOPTS_VALUE)
        {
            pts = frame->pkt_dts;
        }
        else if (frame->pts != AV_NOPTS_VALUE)
        {
            pts = frame->pts;
        }
        else
        {
            pts = 0;
        }
        render->append_samples(m_scale_buffer,scale_buffer_size,pts);
    }
    m_frame_queue->remove_frame_2();
    m_track->notify(); // 通知track
    return 0;
}

void AudioFrameScaler::clean_func(){
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    m_scale_sample_queue->clear();
}

bool AudioFrameScaler::pause_condition(int work_state){
    return m_frame_queue->is_empty() || m_scale_sample_queue->is_full();
}
