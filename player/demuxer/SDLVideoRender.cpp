#include "SDLVideoRender.h"
#include "VideoFrameScaler.h"

int SDLVideoRender::init(){
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        qDebug("SDL2初始化失败 - %s", SDL_GetError());
        return -1;
    }
    else
    {
        auto window=SDL_CreateWindowFrom((void*)(m_widget->winId()));
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
        m_window=std::shared_ptr<SDL_Window>(window,[](SDL_Window * window_ptr){
            SDL_DestroyWindow(window_ptr);
        });
        auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        m_renderer=std::shared_ptr<SDL_Renderer>(renderer,[](SDL_Renderer * renderer_ptr){
            SDL_DestroyRenderer(renderer_ptr);
        });
        auto texture = SDL_CreateTexture(renderer, DST_PIX_FORMAT, SDL_TEXTUREACCESS_STREAMING, m_screen_width, m_screen_height);
        m_texture=std::shared_ptr<SDL_Texture>(texture,[](SDL_Texture * texture_ptr){
            SDL_DestroyTexture(texture_ptr);
        });
        m_rect.x = (window_width - m_screen_height) / 2;
        m_rect.y = (window_height - m_screen_height) / 2;
        m_rect.w = m_screen_height;
        m_rect.h = m_screen_height;
        return VRender::init();
    }
}

int SDLVideoRender::work_func(){
    AVFrame *frame = m_frame_queue->read_frame_2();
    long pts;
    if(frame->pkt_dts != AV_NOPTS_VALUE) {
        pts = frame->pkt_dts;
    } else if (frame->pts != AV_NOPTS_VALUE) {
        pts = frame->pts;
    } else {
        pts = 0;
    }
    pts = (int64_t)((pts * av_q2d(m_time_base)) * 1000);
    long clock=m_clock.get_clock();
    if(clock!=-1){
        if(clock-pts>0){
            m_frame_scaler->render_finish();
            return 0;
        }else{
            std::this_thread::sleep_for(std::chrono::milliseconds(pts-clock));
        }
    }
    int result = SDL_UpdateYUVTexture(m_texture.get(), &m_rect, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
    result = SDL_RenderCopy(m_renderer.get(), m_texture.get(), nullptr, &m_rect);
    if (result >= 0){
        SDL_RenderPresent(m_renderer.get());
        m_clock.set_clock(pts);
    }else{
        return -1;
    }
    m_frame_scaler->render_finish();
    return 0;
}

void SDLVideoRender::resize_window(){
    // 使用资源锁
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
    //更新sws上下文
    m_frame_scaler->init_sws_context(m_screen_width,m_screen_height);
}