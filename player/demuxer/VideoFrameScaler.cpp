#include "VideoFrameScaler.h"
#include "Track.h"
#include "VRender.h"

void VideoFrameScaler::init_sws_context(int screen_width, int screen_height)
{
    // 使用资源锁
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    m_screen_width = screen_width;
    m_screen_height = screen_height;

    SwsContext *sws_context = sws_getContext(m_video_width, m_video_height, m_src_pix_format,
                                             screen_width, screen_height, DST_PIXEL_FORMAT,
                                             SWS_FAST_BILINEAR, NULL, NULL, NULL);
    m_sws_context = std::shared_ptr<SwsContext>(sws_context, [](SwsContext *sws_context_ptr)
                                                {
        if(sws_context_ptr != nullptr) {
            sws_freeContext(sws_context_ptr);
        } });
    m_scale_frame_queue->clear(); // 处理帧队列清空
    m_frame_queue->rollback();    // 原始帧读指针回滚
}

int VideoFrameScaler::init()
{
    Scaler::init();
    // qDebug()<<"VideoDecoder::OnDecoderReady";
    m_video_width = m_track->get_av_codec_context()->width;
    m_video_height = m_track->get_av_codec_context()->height;
    m_src_pix_format = m_track->get_av_codec_context()->pix_fmt;

    if (m_thread_video_render != nullptr)
    {
        std::shared_ptr<VideoFrameScaler> scaler = std::static_pointer_cast<VideoFrameScaler>(shared_from_this());
        m_thread_video_render->set_video_frame_scaler(scaler);
        m_video_render = m_thread_video_render;
        add_thread(m_thread_video_render);
        return m_thread_video_render->init();
    }
    else
    {
        return -1;
    }
}

void VideoFrameScaler::uninit()
{
    m_thread_video_render = nullptr; // 释放强指针
    ThreadChain::uninit();
}

int VideoFrameScaler::work_func()
{
    if(m_frame_queue->is_empty())
    {
        return 0;
    }
    std::shared_ptr<AVFrame> frame = m_frame_queue->read_frame();
    AVFrame *scale_frame = av_frame_alloc();
    scale_frame->width = m_screen_width;
    scale_frame->height = m_screen_height;
    scale_frame->format = DST_PIXEL_FORMAT;
    av_frame_get_buffer(scale_frame, 1);
    std::shared_ptr<AVFrame> frame_ptr=std::shared_ptr<AVFrame>(scale_frame, [](AVFrame *ptr)
        {
            if (ptr != nullptr)
                av_frame_free(&ptr);
        });
    if (sws_scale(m_sws_context.get(), frame->data, frame->linesize, 0,
                  m_video_height, scale_frame->data, scale_frame->linesize) == 0)
    {
        if (auto render = m_video_render.lock())
        {
            render->append_frame(frame_ptr);
        }
        m_frame_queue->remove_frame_2();
        m_track->notify(); // 通知track
        return 0;
    }
    return -1;
}
