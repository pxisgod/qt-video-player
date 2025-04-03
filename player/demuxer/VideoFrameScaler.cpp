#include "VideoFrameScaler.h"
#include "Track.h"
void VideoFrameScaler::init_sws_context(int screen_width, int screen_height)
{
    // 使用资源锁
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    m_screen_width = screen_width;
    m_screen_height = screen_height;

    SwsContext *sws_context = sws_getContext(m_video_width, m_video_height, m_src_pix_format,
                                             screen_width, screen_height, DST_PIXEL_FORMAT,
                                             SWS_FAST_BILINEAR, NULL, NULL, NULL);
    m_sws_context = std::make_shared<SwsContext>(sws_context, [](SwsContext *sws_context_ptr)
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
    AVFrame *frame = m_frame_queue->read_frame_1();
    AVFrame *scale_frame = av_frame_alloc();
    int buffer_size = av_image_get_buffer_size(DST_PIXEL_FORMAT, m_screen_width, m_screen_height, 1);
    uint8_t *frame_buffer = (uint8_t *)av_malloc(buffer_size * sizeof(uint8_t));
    std::unique_ptr<AVFrame> frame_ptr = std::make_unique<AVFrame>(
        frame, [frame_buffer](AVFrame *ptr)
        {
        if(ptr!=nullptr)
                    av_frame_free(&ptr); 
                    av_free(frame_buffer); });
    if (sws_scale(m_sws_context.get(), frame->data, frame->linesize, 0,
                  m_video_height, scale_frame->data, scale_frame->linesize) == 0)
    {
        if (auto render = m_video_render.lock())
        {
            render->append_frame(std::move(frame_ptr));
        }
        m_track->notify(); // 通知track
        return 0;
    }
    return -1;
}
