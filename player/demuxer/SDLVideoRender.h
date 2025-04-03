#ifndef SDL_VIDEO_RENDER_H
#define SDL_VIDEO_RENDER_H
#include "VRender.h"
#include <SDL2/SDL.h>
#include <QWidget>
class SDLVideoRender:public VRender{
public:
    explicit SDLVideoRender(QWidget *widget):m_widget(widget){
        SDL_Quit();//释放资源
    }
    virtual ~SDLVideoRender(){}
    virtual int init();
    virtual int work_func();
    virtual void resize_window();
    const SDL_PixelFormatEnum DST_PIX_FORMAT=SDL_PIXELFORMAT_IYUV;
private:
    QWidget *m_widget;
    std::shared_ptr<SDL_Window> m_window ;
    std::shared_ptr<SDL_Renderer> m_renderer;  // 渲染器
    std::shared_ptr<SDL_Texture> m_texture; 
    SDL_Rect m_rect;
};
#endif