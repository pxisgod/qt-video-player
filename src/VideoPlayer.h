#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QWidget>
#include "FFMediaPlayer.h"

enum PauseState{
    PAUSE,
    PLAYING,
    STOP,
    SWITCHING
};

class VideoPlayer : public QObject {
    Q_OBJECT

public:
    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer(){
        uninit();
    }

    void init(const QString &filePath);
    void uninit();
    void setVideoOutput(QWidget *videoWidget); // 设置播放窗口

    void play();  // 播放
    void pause(); // 暂停
    void seek(qint64 position); // 跳转到指定位置
    void resize(int width,int height); //修改窗口大小
    int getPauseState(){
        return pauseState;
    }
    void setPauseState(int pauseState){
        this->pauseState=pauseState;
    }

signals:
    void decoderInitError();
    void decoderInitReady(long duration);
    void decodeTimeChanged(long decodeTime); // 当前编码时间信号
    void renderTimeChanged(long renderTime); // 当前渲染时间信号
    void pauseTime(long pauseTime);
    void playDone();
    void decoderDone();
    void exception();//发生异常

private:
    static void callback(void * context,int type,float time);
    void emitSignal(int type,float time);

private:
    FFMediaPlayer *mediaPlayer=nullptr;
    QWidget *widget=nullptr;
    int pauseState;
};

#endif // VIDEOPLAYER_H
