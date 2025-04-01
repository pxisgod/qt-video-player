#include "Sdl2VideoRender.h"
#include <qdebug.h>

Sdl2VideoRender::Sdl2VideoRender(QWidget *widget) : VideoRender(VIDEO_RENDER_ANWINDOW),widget(widget)
{

}

Sdl2VideoRender::~Sdl2VideoRender()
{
}

void Sdl2VideoRender::Init(int videoWidth, int videoHeight, int *dstSize)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        //qDebug("SDL2初始化失败 - %s", SDL_GetError());
    }
    else
    {
        //m_window = SDL_CreateWindow("test",  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 480, 640, SDL_WINDOW_SHOWN);
        m_window=SDL_CreateWindowFrom((void*)(widget->winId()));
        int windowWidth = widget->width();
        int windowHeight = widget->height();

        if (windowWidth < windowHeight * videoWidth / videoHeight)
        {
            m_DstWidth = windowWidth;
            m_DstHeight = windowWidth * videoHeight / videoWidth;
        }
        else
        {
            m_DstWidth = windowHeight * videoWidth / videoHeight;
            m_DstHeight = windowHeight;
        }
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);

        m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_DstWidth, m_DstHeight);

        m_rect.x = (windowWidth - m_DstWidth) / 2;
        m_rect.y = (windowHeight - m_DstHeight) / 2;
        m_rect.h = m_DstHeight;
        m_rect.w = m_DstWidth;

        dstSize[0] = m_DstWidth;
        dstSize[1] = m_DstHeight;
    }
}

void Sdl2VideoRender::RenderVideoFrame(NativeImage *pImage)
{
    int result = SDL_UpdateYUVTexture(m_texture, &m_rect, pImage->ppPlane[0], pImage->pLineSize[0], pImage->ppPlane[1], pImage->pLineSize[1], pImage->ppPlane[2], pImage->pLineSize[2]);
    result = SDL_RenderCopy(m_renderer, m_texture, nullptr, &m_rect);
    if (result >= 0)
        SDL_RenderPresent(m_renderer);
}

void Sdl2VideoRender::UnInit()
{
    // 销毁
    if (m_renderer != nullptr)
    {
        SDL_DestroyTexture(m_texture);
    }
    if (m_renderer != nullptr)
    {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window != nullptr)
    {
        SDL_DestroyWindow(m_window);
    }
    // 释放资源
    SDL_Quit();
}
