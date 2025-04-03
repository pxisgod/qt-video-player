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
            screen_width = window_width;
            screen_height = window_width * m_frame_scaler->get_video_height() / m_frame_scaler->get_video_width();
        }
        else
        {
            screen_width = window_height * m_frame_scaler->get_video_width() / m_frame_scaler->get_video_height();
            screen_height = window_height;
        }
        m_window=std::make_shared<SDL_Window>(window,[](SDL_Window * window_ptr){
            SDL_DestroyWindow(window_ptr);
        });
        auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        m_renderer=std::make_shared<SDL_Renderer>(renderer,[](SDL_Renderer * renderer_ptr){
            SDL_DestroyRenderer(renderer_ptr);
        });
        auto texture = SDL_CreateTexture(renderer, DST_PIX_FORMAT, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
        m_texture=std::make_shared<SDL_Texture>(texture,[](SDL_Texture * texture_ptr){
            SDL_DestroyTexture(texture_ptr);
        });
        m_rect.x = (window_width - screen_width) / 2;
        m_rect.y = (window_height - screen_height) / 2;
        m_rect.w = screen_width;
        m_rect.h = screen_height;
        return VRender::init();
    }
}

int SDLVideoRender::work_func(){

    m_frame_scaler->render_finish();
}

void SDLVideoRender::resize_window(){

    // 使用资源锁
    std::lock_guard<std::mutex> lock(m_rsc_mutex);
    int window_width = m_widget->width();
    int window_height = m_widget->height();

    if (window_width < window_height * m_frame_scaler->get_video_width() / m_frame_scaler->get_video_height())
    {
        screen_width = window_width;
        screen_height = window_width * m_frame_scaler->get_video_height() / m_frame_scaler->get_video_width();
    }
    else
    {
        screen_width = window_height * m_frame_scaler->get_video_width() / m_frame_scaler->get_video_height();
        screen_height = window_height;
    }
    m_window=std::make_shared<SDL_Window>(window,[](SDL_Window * window_ptr){
        SDL_DestroyWindow(window_ptr);
    });
    auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    m_renderer=std::make_shared<SDL_Renderer>(renderer,[](SDL_Renderer * renderer_ptr){
        SDL_DestroyRenderer(renderer_ptr);
    });
    auto texture = SDL_CreateTexture(renderer, DST_PIX_FORMAT, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
    m_texture=std::make_shared<SDL_Texture>(texture,[](SDL_Texture * texture_ptr){
        SDL_DestroyTexture(texture_ptr);
    });
    m_rect.x = (window_width - screen_width) / 2;
    m_rect.y = (window_height - screen_height) / 2;
    m_rect.w = screen_width;
    m_rect.h = screen_height;
    //更新sws上下文
    m_frame_scaler->init_sws_context(screen_width,screen_height);
}