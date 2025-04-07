#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QWidget>
#include "FFMediaPlayer.h"
#include "Demuxer.h"

class VideoPlayer : public QObject,public EventListener<ThreadMsg>{
    Q_OBJECT

public:
    explicit VideoPlayer(QObject *parent = nullptr):QObject(parent){
        m_demuxer=std::make_shared<Demuxer>();
        m_demuxer->add_listener(this); //添加到消息链中
    }
    ~VideoPlayer(){
    }

    void setVideoOutput(QWidget *widget); // 设置播放窗口

    int play(const QString &file_path);    // 开始播放
    int play();                            // 继续播放
    int pause();                           // 暂停
    int stop();                            // 停止
    int seek(qint64 position);             // 跳转到指定位置
    int resize(int width,int height);      //修改窗口大小
    virtual void deal_event(ThreadMsg event); //消息处理
    int get_play_state(){
        return m_demuxer->get_play_state();
    }

signals:
    void init_done(long duration);//获取时长
    void switch_playing();
    void switch_pause();
    void switch_stop(); 

private:
    std::shared_ptr<Demuxer> m_demuxer;
    QWidget *m_widget;
};

#endif // VIDEOPLAYER_H
