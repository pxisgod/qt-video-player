#include "VideoPlayerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMediaPlaylist>
#include <QLabel>
#include <QMainWindow>


VideoPlayerWidget::VideoPlayerWidget(QWidget *parent)
    : QWidget(parent),
      mediaPlayer(new VideoPlayer(this)),
      videoWidget(new QMainWindow(this)),
      progressSlider(new QSlider(Qt::Horizontal, this)),
      playPauseButton(new QPushButton("Play", this)),
      playlist(new QListWidget(this)) {

    setupUI();

    // Connect signals and slots
    connect(playPauseButton, &QPushButton::clicked, this, &VideoPlayerWidget::playPause);
    connect(progressSlider, &QSlider::sliderMoved, this, &VideoPlayerWidget::setPosition);
    connect(playlist, &QListWidget::itemDoubleClicked, this, &VideoPlayerWidget::handleItemDoubleClicked);


    connect(mediaPlayer, &VideoPlayer::decoderInitError, this, [this]() {
        mediaPlayer->uninit();
        mediaPlayer->setPauseState(STOP);
    });
    connect(mediaPlayer, &VideoPlayer::decoderInitReady, this, [this](long duration) {
        progressSlider->setRange(0,duration);
        progressSlider->setValue(0);
    });
    connect(mediaPlayer, &VideoPlayer::renderTimeChanged, this, [this](long renderTime){
        progressSlider->setValue(static_cast<int>(renderTime));
    });
    /**
    connect(mediaPlayer, &VideoPlayer::pauseTime, this, [this](long pauseTime) {
        progressSlider->setValue(static_cast<int>(pauseTime));
    });
     */
    connect(mediaPlayer, &VideoPlayer::playDone, this, [this]() {
        if(mediaPlayer->getPauseState()==PLAYING){
            mediaPlayer->uninit();
            mediaPlayer->setPauseState(STOP);
            playPauseButton->setText("Stop");
            switch (loopType)
            {
            case LIST_NEXT:
                //获取列表中下一项播放
                break;
            default:
                break;
            }
        }
    });
    /**
    connect(mediaPlayer, &VideoPlayer::decoderDone, this,[this](){

    });
    */
    

    loadPlaylist();
}

VideoPlayerWidget::~VideoPlayerWidget() {}

void VideoPlayerWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Video display
    mediaPlayer->setVideoOutput(videoWidget);
    mainLayout->addWidget(videoWidget);

    // Playback controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(playPauseButton);
    controlsLayout->addWidget(progressSlider);
    mainLayout->addLayout(controlsLayout);

    // Playlist
    mainLayout->addWidget(playlist);

    setLayout(mainLayout);
    videoWidget->show();
}

void VideoPlayerWidget::loadPlaylist() {
    //todo: 加载保存的播放列表
}

void VideoPlayerWidget::addToPlaylist(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName(); // 获取文件名

    QListWidgetItem *item = new QListWidgetItem(fileName);
    item->setData(Qt::UserRole, filePath); // 保存实际文件路径
    playlist->addItem(item);
}

void VideoPlayerWidget::playPause() {
    if (mediaPlayer->getPauseState() == PLAYING) {
        mediaPlayer->pause();
        playPauseButton->setText("Play");
    } else if(mediaPlayer->getPauseState() == PAUSE){
        mediaPlayer->play();
        playPauseButton->setText("Pause");
    }else{

    }
}

void VideoPlayerWidget::setPosition(int position) {
    mediaPlayer->seek(position);
}

void VideoPlayerWidget::handleItemDoubleClicked(QListWidgetItem *item) {
    mediaPlayer->uninit();
    // 从 QListWidgetItem 的 data 中获取实际文件路径
    QString filePath = item->data(Qt::UserRole).toString();
    mediaPlayer->init(filePath);
    mediaPlayer->play();
    playPauseButton->setText("Pause");
}

void VideoPlayerWidget::handleMediaEnd() {
    int currentRow = playlist->currentRow();
    if (currentRow + 1 < playlist->count()) {
        playlist->setCurrentRow(currentRow + 1);
        handleItemDoubleClicked(playlist->currentItem());
    }
}