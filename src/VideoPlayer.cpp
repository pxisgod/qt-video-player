#include "VideoPlayer.h"
#include "SDLVideoRender.h"
#include "VideoFrameScaler.h"
#include <QDebug>

void VideoPlayer::setVideoOutput(QWidget *widget)
{
    this->m_widget = widget;
}

int VideoPlayer::play(const QString &file_path)
{
    int ret = -1;
    std::shared_ptr<SDLVideoRender> videoRender = std::make_shared<SDLVideoRender>(m_widget);
    VideoFrameScaler::set_thread_render(videoRender);
    if (m_demuxer->init_0(file_path.toStdString()) == 0)
    {
        m_demuxer->start_0();
        ret = 0;
    }
    VideoFrameScaler::remove_thread_render();
    return ret;
}
int VideoPlayer::play()
{
    return m_demuxer->play_0();
}

int VideoPlayer::pause()
{
    return m_demuxer->pause_0();
}

int VideoPlayer::stop()
{
    return m_demuxer->stop_0();
}

int VideoPlayer::seek(qint64 position)
{
    return m_demuxer->seek_0(position);
}

int VideoPlayer::resize(int width, int height)
{
    // todo:
    return 0;
}

void VideoPlayer::deal_event(ThreadMsg event)
{
    switch (event.m_msg_type)
    {
    case INIT_DONE:
        emit init_done(event.m_msg_time/1000); //转换成秒
        break;
    case SWITCH_PLAYING:
        emit switch_playing();
        break;
    case SWITCH_PAUSE:
        emit switch_pause();
        break;
    case SWITCH_STOP:
        emit switch_stop();
        break;
    default:
        break;
    };
}