/**
 *
 * Created by 公众号：字节流动 on 2021/3/16.
 * https://github.com/githubhaohao/LearnFFmpeg
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */


#ifndef LEARNFFMPEG_FFMEDIAPLAYER_H
#define LEARNFFMPEG_FFMEDIAPLAYER_H

#include <MediaPlayer.h>


class FFMediaPlayer : public MediaPlayer {
public:
    FFMediaPlayer(){};
    virtual ~FFMediaPlayer(){
        UnInit();
    };

public:
    virtual void Init(const char *url,int videoRenderType,void * context,MessageCallback messageCallback);
    virtual void UnInit();

    virtual void Play();
    virtual void Pause();
    virtual void Resize(int width,int height);
    virtual void SeekToPosition(float position);
    virtual long GetMediaParams(int paramType);

private:

    static void PostMessage(void *context, int msgType, float msgCode);

    VideoDecoder *m_VideoDecoder = nullptr;
    AudioDecoder *m_AudioDecoder = nullptr;

    VideoRender *m_VideoRender = nullptr;
    AudioRender *m_AudioRender = nullptr;
};


#endif //LEARNFFMPEG_FFMEDIAPLAYER_H
