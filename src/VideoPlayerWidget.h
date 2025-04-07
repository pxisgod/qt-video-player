#ifndef VIDEOPLAYERWIDGET_H
#define VIDEOPLAYERWIDGET_H

#include <QWidget>
#include <QVideoWidget>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include <QMediaPlayer>
#include "VideoPlayer.h"
#include "ProgressWidget.h"


class VideoPlayerWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget *parent = nullptr);
    ~VideoPlayerWidget();
    void add_to_play_list(const QString &file_path); // 添加文件到播放列表
private slots:
    void play_or_pause() ;
    void button_handle_play() ;
    void button_handle_pause() ;
    void button_handle_stop() ;
    void handle_item_double_clicked(QListWidgetItem *item) ;
private:
    VideoPlayer *m_video_player;
    QWidget *m_widget;
    ProgressWidget *m_progress_slider;
    QPushButton *m_play_button;
    QListWidget *m_play_list;

    void setupUI();
};

#endif // VIDEOPLAYERWIDGET_H