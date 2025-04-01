/**
 *
 * Created by 公众号：字节流动 on 2021/3/16.
 * https://github.com/githubhaohao/LearnFFmpeg
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */
#include "Sdl2VideoRender.h"
#include "Sdl2AudioRender.h"
#include "FFMediaPlayer.h"
#include <qdebug.h>

void FFMediaPlayer::Init(const char *url,int videoRenderType,void * context,MessageCallback messageCallback)
{
    m_VideoDecoder = new VideoDecoder(url);
    m_AudioDecoder = new AudioDecoder(url);

    if (videoRenderType == VIDEO_RENDER_ANWINDOW)
    {
        if(widget!=nullptr){
            m_VideoRender = new Sdl2VideoRender(widget);
            m_VideoDecoder->SetVideoRender(m_VideoRender);
        }else{
            throw std::invalid_argument("缺失widget");
        }
    }

    m_AudioRender = new Sdl2AudioRender();
    m_AudioDecoder->SetAudioRender(m_AudioRender);

    m_VideoDecoder->SetMessageCallback(context, messageCallback);
    m_AudioDecoder->SetMessageCallback(context, messageCallback);
}

void FFMediaPlayer::UnInit()
{
    //qDebug("FFMediaPlayer::UnInit");
    if (m_VideoDecoder)
    {
        delete m_VideoDecoder;
        m_VideoDecoder = nullptr;
    }

    if (m_VideoRender)
    {
        delete m_VideoRender;
        m_VideoRender = nullptr;
    }

    if (m_AudioDecoder)
    {
        delete m_AudioDecoder;
        m_AudioDecoder = nullptr;
    }

    if (m_AudioRender)
    {
        delete m_AudioRender;
        m_AudioRender = nullptr;
    }
    
}

void FFMediaPlayer::Play()
{
    //qDebug("FFMediaPlayer::Play");
    if (m_VideoDecoder)
        m_VideoDecoder->Start();

    if (m_AudioDecoder)
        m_AudioDecoder->Start();
}

void FFMediaPlayer::Pause()
{
    //qDebug("FFMediaPlayer::Pause");
    if (m_VideoDecoder)
        m_VideoDecoder->Pause();

    if (m_AudioDecoder)
        m_AudioDecoder->Pause();
}

void FFMediaPlayer::Resize(int width,int height){

}



void FFMediaPlayer::SeekToPosition(float position)
{
    //qDebug("FFMediaPlayer::SeekToPosition position=%f", position);
    if (m_VideoDecoder)
        m_VideoDecoder->SeekToPosition(position);

    if (m_AudioDecoder)
        m_AudioDecoder->SeekToPosition(position);
}

long FFMediaPlayer::GetMediaParams(int paramType)
{
    ("FFMediaPlayer::GetMediaParams paramType=%d", paramType);
    long value = 0;
    switch (paramType)
    {
    case MEDIA_PARAM_VIDEO_WIDTH:
        value = m_VideoDecoder != nullptr ? m_VideoDecoder->GetVideoWidth() : 0;
        break;
    case MEDIA_PARAM_VIDEO_HEIGHT:
        value = m_VideoDecoder != nullptr ? m_VideoDecoder->GetVideoHeight() : 0;
        break;
    case MEDIA_PARAM_VIDEO_DURATION:
        value = m_VideoDecoder != nullptr ? m_VideoDecoder->GetDuration() : 0;
        break;
    }
    return value;
}

