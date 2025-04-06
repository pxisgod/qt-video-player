#include <QtTest/QtTest>
#include <Demuxer.h>
#include <SDLVideoRender.h>
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QDebug>
#include <VideoFrameScaler.h>

class TestDemuxer : public QObject {
    Q_OBJECT

private slots:
    void testDemuxer() {
        int argc=0;
        QApplication app(argc, nullptr);
 
        QMainWindow *window=new QMainWindow();
        window->resize(680, 360); // 设置窗口大小
        window->setWindowTitle("Video Player"); // 设置窗口标题
        window->show(); // 显示窗口

        std::shared_ptr<Demuxer> demuxer = std::make_shared<Demuxer>("/home/px/Code/VideoPlayer/qt-video-player/27791853791-1-16.mp4");
        std::shared_ptr<SDLVideoRender> videoRender = std::make_shared<SDLVideoRender>(window);
        VideoFrameScaler::set_thread_render(videoRender);
        demuxer->init_0();
        demuxer->start_0();

        app.exec();
    }
};

QTEST_MAIN(TestDemuxer)
#include "TestDemuxer.moc"
