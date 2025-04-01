/**
 *
 * Created by 公众号：字节流动 on 2021/12/16.
 * https://github.com/githubhaohao/LearnFFmpeg
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#ifndef LEARNFFMPEG_MEDIAPLAYER_H
#define LEARNFFMPEG_MEDIAPLAYER_H
#include <VideoDecoder.h>
#include <AudioDecoder.h>
#include <AudioRender.h>
#include <VideoRender.h>
#include <QWidget>

#define JAVA_PLAYER_EVENT_CALLBACK_API_NAME "playerEventCallback"

#define MEDIA_PARAM_VIDEO_WIDTH         0x0001
#define MEDIA_PARAM_VIDEO_HEIGHT        0x0002
#define MEDIA_PARAM_VIDEO_DURATION      0x0003

#define MEDIA_PARAM_ASSET_MANAGER       0x0020


class MediaPlayer {
public:
    MediaPlayer(){};
    virtual ~MediaPlayer(){};
    void setWidget(QWidget *widget){
        this->widget=widget;
    }

    virtual void Init(const char *url,int videoRenderType,void * context,MessageCallback messageCallback)=0;
    virtual void UnInit() = 0;

public:
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Resize(int width,int height)=0;
    virtual void SeekToPosition(float position) = 0;
    virtual long GetMediaParams(int paramType) = 0;
    virtual void SetMediaParams(int paramType){}
protected:
    QWidget *widget;

};

#endif //LEARNFFMPEG_MEDIAPLAYER_H
