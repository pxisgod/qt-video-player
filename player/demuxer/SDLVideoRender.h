#ifndef SDL_VIDEO_RENDER_H
#define SDL_VIDEO_RENDER_H
#include "VRender.h"
#include <QWidget>
#include <SDLRender.h>
class SDLVideoRender:public VRender,public SDL_Render{
public:
    explicit SDLVideoRender(QWidget *widget):m_widget(widget){
        
    }
    virtual ~SDLVideoRender(){

    }
    virtual int init();
    virtual int thread_init();
    virtual int work_func();
    virtual void resize_window();
    virtual bool pause_condition();
    virtual long get_wait_time();
    virtual void deal_after_wait();
    virtual void deal_neg_wait_time();
    virtual void adjust_clock(long position) ; //调整时钟
    virtual void adjust_clock() ; //调整时钟
    const SDL_PixelFormatEnum DST_PIX_FORMAT=SDL_PIXELFORMAT_IYUV;
private:
    QWidget *m_widget;
    std::shared_ptr<SDL_Window> m_window ;
    std::shared_ptr<SDL_Renderer> m_renderer;  // 渲染器
    std::shared_ptr<SDL_Texture> m_texture; 
    static thread_local std::shared_ptr<AVFrame> m_pending_frame;
    static thread_local double m_sleep_time;
    static thread_local long m_frame_pts;
    SDL_Rect m_rect;
};
#endif