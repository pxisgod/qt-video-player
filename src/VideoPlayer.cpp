#include "VideoPlayer.h"
#include <QDebug>

VideoPlayer::VideoPlayer(QObject *parent)
    : QObject(parent), pauseState(STOP) {}

void VideoPlayer::init(const QString &filePath) {
    mediaPlayer = new FFMediaPlayer();
    if(widget){
        mediaPlayer->setWidget(widget);
    }
    mediaPlayer->Init(filePath.toStdString().data(),VIDEO_RENDER_ANWINDOW,this,callback);
}

void VideoPlayer::uninit() {
    if (mediaPlayer!=nullptr) {
        mediaPlayer->UnInit();
        delete mediaPlayer;
        mediaPlayer = nullptr;
    }
}

void VideoPlayer::setVideoOutput(QWidget *videoWidget) {
    this->widget=videoWidget;
}

void VideoPlayer::play() {
    if (mediaPlayer!=nullptr) {
        mediaPlayer->Play();
        pauseState = PLAYING;
    }
}

void VideoPlayer::pause() {
    if (mediaPlayer!=nullptr) {
        mediaPlayer->Pause();
        pauseState=PAUSE;
    }
}

void VideoPlayer::seek(qint64 position) {
    if (mediaPlayer!=nullptr) {
        mediaPlayer->SeekToPosition(position);
    }
}

void VideoPlayer::resize(int width, int height) {
    if (mediaPlayer!=nullptr) {
        mediaPlayer->Resize(width, height);
    }
}


void VideoPlayer::callback(void * context,int type, float time) {
    VideoPlayer *player=static_cast<VideoPlayer *>(context); 
    player->emitSignal(type,time);
}


void VideoPlayer::emitSignal(int type,float time){
    switch (type) {
        case MSG_DECODER_INIT_ERROR:
            emit decoderInitError();
            break;
        case MSG_DECODER_READY:
            emit decoderInitReady(time);
            break;
        case MSG_DECODER_DONE:
            emit decoderDone();
            break;
        case MSG_DECODING_TIME:
            emit decodeTimeChanged(time);
            break;
        case MSG_RENDER_TIME:
            emit renderTimeChanged(time);
            break;
        case MSG_PAUSE_TIME:
            emit pauseTime(time);
            break;
        case MSG_PLAY_DONE:
            emit playDone();
            break;
        default:
            //qDebug() << "Unknown callback type:" << type;
            break;
    }
}

