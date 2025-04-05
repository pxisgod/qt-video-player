#include "SDLVideoRender.h"
#include "VideoFrameScaler.h"

int SDLVideoRender::init()
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        qDebug("SDL2初始化失败 - %s", SDL_GetError());
        return -1;
    }
    else
    {
        auto window = SDL_CreateWindowFrom((void *)(m_widget->winId()));
        int window_width = m_widget->width();
        int window_height = m_widget->height();

        if (window_width < window_height * m_frame_scaler->get_video_width() / m_frame_scaler->get_video_height())
        {
            m_screen_width = window_width;
            m_screen_height = window_width * m_frame_scaler->get_video_height() / m_frame_scaler->get_video_width();
        }
        else
        {
            m_screen_width = window_height * m_frame_scaler->get_video_width() / m_frame_scaler->get_video_height();
            m_screen_height = window_height;
        }
        m_window = std::shared_ptr<SDL_Window>(window, [](SDL_Window *window_ptr)
                                               { SDL_DestroyWindow(window_ptr); });
        auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        m_renderer = std::shared_ptr<SDL_Renderer>(renderer, [](SDL_Renderer *renderer_ptr)
                                                   { SDL_DestroyRenderer(renderer_ptr); });
        auto texture = SDL_CreateTexture(renderer, DST_PIX_FORMAT, SDL_TEXTUREACCESS_STREAMING, m_screen_width, m_screen_height);
        m_texture = std::shared_ptr<SDL_Texture>(texture, [](SDL_Texture *texture_ptr)
                                                 { SDL_DestroyTexture(texture_ptr); });
        m_rect.x = (window_width - m_screen_height) / 2;
        m_rect.y = (window_height - m_screen_height) / 2;
        m_rect.w = m_screen_height;
        m_rect.h = m_screen_height;
        return VRender::init();
    }
}

int SDLVideoRender::work_func()
{
    if (m_frame_queue->is_empty())
    {
        return 0;
    }
    std::shared_ptr<AVFrame> frame = m_frame_queue->read_frame();
    if (m_pending_frame != nullptr && frame == m_pending_frame)
    {

        int result = SDL_UpdateYUVTexture(m_texture.get(), &m_rect, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
        result = SDL_RenderCopy(m_renderer.get(), m_texture.get(), nullptr, &m_rect);
        if (result >= 0)
        {
            SDL_RenderPresent(m_renderer.get());
            m_clock->set_clock(m_frame_pts); // 设置时钟
        }
        else
        {
            return -1;
        }
        m_frame_queue->remove_frame_3();
        m_frame_scaler->render_finish();
    }
    return 0;
}

void SDLVideoRender::resize_window()
{
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    int window_width = m_widget->width();
    int window_height = m_widget->height();

    if (window_width < window_height * m_frame_scaler->get_video_width() / m_frame_scaler->get_video_height())
    {
        m_screen_width = window_width;
        m_screen_height = window_width * m_frame_scaler->get_video_height() / m_frame_scaler->get_video_width();
    }
    else
    {
        m_screen_width = window_height * m_frame_scaler->get_video_width() / m_frame_scaler->get_video_height();
        m_screen_height = window_height;
    }
    m_rect.x = (window_width - m_screen_width) / 2;
    m_rect.y = (window_height - m_screen_height) / 2;
    m_rect.w = m_screen_width;
    m_rect.h = m_screen_height;
    // 更新sws上下文
    m_frame_scaler->init_sws_context(m_screen_width, m_screen_height);
}

bool SDLVideoRender::pause_condition()
{
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    if (m_frame_queue->is_empty())
    {
        return true;
    }
    std::shared_ptr<AVFrame> frame = m_frame_queue->read_frame();
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
    double sleep_time = m_clock->get_sleep_time(pts);
    // 设置线程变量
    m_pending_frame = frame;
    m_sleep_time = sleep_time;
    m_frame_pts = pts;

    if (sleep_time != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
long SDLVideoRender::get_wait_time()
{
    return m_sleep_time;
}

void SDLVideoRender::deal_neg_wait_time()
{
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    if (m_frame_queue->is_empty())
    {
        return;
    }
    std::shared_ptr<AVFrame> frame = m_frame_queue->read_frame();
    if (m_pending_frame != nullptr && frame == m_pending_frame)
    {
        m_frame_queue->remove_frame_3();
        m_frame_scaler->render_finish();
    }
    m_sleep_time = 0;
}