
#ifndef SDL2_VIDEORENDER_H
#define SDL2_VIDEORENDER_H
#include "VideoRender.h"
#include <SDL2/SDL.h>
#include <QWidget>
#include "util/ImageDef.h"


class Sdl2VideoRender : public VideoRender{

public:
    Sdl2VideoRender(QWidget *widget);
    virtual ~Sdl2VideoRender();
    virtual void Init(int videoWidth, int videoHeight, int *dstSize);
    virtual void RenderVideoFrame(NativeImage *pImage);
    virtual void UnInit();

private:
    int m_DstWidth;
    int m_DstHeight;
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_renderer = nullptr;  // 渲染器
    SDL_Texture *m_texture = nullptr; 
    SDL_Rect m_rect;
    QWidget *widget;
};
#endif
