#ifndef VIDEOPLAYERWIDGET_H
#define VIDEOPLAYERWIDGET_H

#include <QWidget>
#include <QVideoWidget>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include <QMediaPlayer>
#include "VideoPlayer.h"

enum LoopType{
    LIST_NEXT,
    ONE_FINISH,
    ONE_CIRCLE,
    LIST_CIRCLE
};

class VideoPlayerWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget *parent = nullptr);
    ~VideoPlayerWidget();

    void addToPlaylist(const QString &filePath); // 添加文件到播放列表

private slots:
    void playPause();
    void setPosition(int position);
    void handleItemDoubleClicked(QListWidgetItem *item);
    void handleMediaEnd();

private:
    VideoPlayer *mediaPlayer;
    QWidget *videoWidget;
    QSlider *progressSlider;
    QPushButton *playPauseButton;
    QListWidget *playlist;

    void setupUI();
    void loadPlaylist();
    int loopType=LIST_NEXT;
};

#endif // VIDEOPLAYERWIDGET_H