#include "VideoPlayerWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMediaPlaylist>
#include <QLabel>
#include <QMainWindow>


VideoPlayerWidget::VideoPlayerWidget(QWidget *parent)
    : QWidget(parent),
      m_video_player(new VideoPlayer(this)),
      m_widget(new QMainWindow(this)),
      m_progress_slider(new ProgressWidget(this)),
      m_play_button(new QPushButton( this)),
      m_play_list(new QListWidget(this)) {
    
    m_play_button->setIcon(QIcon(":/images/stop.png"));//初始设置为停止图标
    m_play_button->setIconSize(QSize(32, 32));
    m_play_button->setEnabled(false);

    setupUI();

    //进度条相关的信号和槽连接
    connect(m_progress_slider, &ProgressWidget::seek, m_video_player, &VideoPlayer::seek);
    connect(m_video_player, &VideoPlayer::init_done, m_progress_slider, &ProgressWidget::set_duration);
    connect(m_video_player, &VideoPlayer::switch_playing, m_progress_slider, &ProgressWidget::start_timer);
    connect(m_video_player, &VideoPlayer::switch_pause, m_progress_slider, &ProgressWidget::stop_timer);
    connect(m_video_player, &VideoPlayer::switch_stop, m_progress_slider, &ProgressWidget::stop_timer);


    //播放列表相关的信号和槽连接
    connect(m_play_list, &QListWidget::itemDoubleClicked, this, &VideoPlayerWidget::handle_item_double_clicked);
    

    //播放按钮相关的信号和槽连接
    connect(m_play_button, &QPushButton::clicked, this, &VideoPlayerWidget::play_or_pause);
    connect(m_video_player, &VideoPlayer::switch_playing, this, &VideoPlayerWidget::button_handle_play);
    connect(m_video_player, &VideoPlayer::switch_pause, this, &VideoPlayerWidget::button_handle_pause);
    connect(m_video_player, &VideoPlayer::switch_stop, this, &VideoPlayerWidget::button_handle_stop);
}

VideoPlayerWidget::~VideoPlayerWidget() {}


//播放按钮的槽函数和函数
void VideoPlayerWidget::play_or_pause() {
    if(m_video_player->get_play_state() == PLAYING) {
        m_video_player->pause();
    } else if(m_video_player->get_play_state() == PAUSE){
        m_video_player->play();
    }
}

void VideoPlayerWidget::button_handle_play(){
    m_play_button->setIcon(QIcon(":/images/playing.png"));
    m_play_button->setIconSize(QSize(32, 32));
    m_play_button->setEnabled(true);
}
void VideoPlayerWidget::button_handle_pause() {
    m_play_button->setIcon(QIcon(":/images/pause.png"));
    m_play_button->setIconSize(QSize(32, 32));
    m_play_button->setEnabled(true);
}
void VideoPlayerWidget::button_handle_stop() {
    m_play_button->setIcon(QIcon(":/images/stop.png"));
    m_play_button->setIconSize(QSize(32, 32));
    m_play_button->setEnabled(false);
}



//播放列表相关的槽和函数
void VideoPlayerWidget::add_to_play_list(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName(); // 获取文件名

    QListWidgetItem *item = new QListWidgetItem(fileName);
    item->setData(Qt::UserRole, filePath); // 保存实际文件路径
    m_play_list->addItem(item);
}

void VideoPlayerWidget::handle_item_double_clicked(QListWidgetItem *item) {
    // 从 QListWidgetItem 的 data 中获取实际文件路径
    QString file_path = item->data(Qt::UserRole).toString();
    if(m_video_player->get_play_state()==STOPPED || m_video_player->stop()==0){
        m_video_player->play(file_path);
    }
}


void VideoPlayerWidget::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this); // 修改为水平布局

    // 左侧布局：视频显示和播放控制
    QVBoxLayout *leftLayout = new QVBoxLayout();

    // Video display
    m_widget->resize(1028, 960); // 设置初始大小
    m_video_player->setVideoOutput(m_widget);
    leftLayout->addWidget(m_widget);

    // Playback controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(m_play_button);
    controlsLayout->addWidget(m_progress_slider);
    leftLayout->addLayout(controlsLayout);

    mainLayout->addLayout(leftLayout); // 添加左侧布局

    // 右侧布局：播放列表
    mainLayout->addWidget(m_play_list); // 播放列表移动到右边

    setLayout(mainLayout);
    m_widget->show();
}

