#include "ProgressWidget.h"
#include <QHBoxLayout>
#include <QTime>
#include <QToolTip>
#include <QCursor>

ProgressWidget::ProgressWidget(QWidget *parent)
    : QWidget(parent), progress_bar(new QSlider(Qt::Horizontal, this)), time_label(new QLabel("00:00:00", this)), timer(new QTimer(this)) {

    // 设置布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(progress_bar);
    layout->addWidget(time_label);
    setLayout(layout);

    // 设置进度条范围和初始值
    progress_bar->setRange(0, 0);
    progress_bar->setValue(0);

    // 连接定时器信号以更新进度条
    connect(timer, &QTimer::timeout, this, &ProgressWidget::update_progress);

    // 处理进度条滑动事件
    connect(progress_bar, &QSlider::sliderMoved, this, [this](int position) {
        emit seek(position);
    });

    // 显示进度条当前位置的时间
    connect(progress_bar, &QSlider::sliderMoved, this, &ProgressWidget::show_tooltip);

    // 隐藏提示
    connect(progress_bar, &QSlider::sliderReleased, this, &ProgressWidget::hide_tooltip);
}

void ProgressWidget::set_duration(int duration_in_seconds) {
    time_label->setText(QTime(0, 0).addSecs(duration_in_seconds).toString("HH:mm:ss"));
    progress_bar->setRange(0, duration_in_seconds);
    timer->start(1000); // 每秒更新一次
}


void ProgressWidget::update_progress() {
    int current_value = progress_bar->value();
    if (current_value < progress_bar->maximum()) {
        progress_bar->setValue(current_value + 1);
    } else {
        timer->stop();
    }
}

void ProgressWidget::show_tooltip(int position) {
    QToolTip::showText(QCursor::pos(), QTime(0, 0).addSecs(position).toString("HH:mm:ss"), progress_bar);
}

void ProgressWidget::hide_tooltip() {
    QToolTip::hideText();
}

void ProgressWidget::start_timer() {
    if (!timer->isActive()) {
        timer->start(1000); // 每秒更新一次
    }
}

void ProgressWidget::stop_timer() {
    if (timer->isActive()) {
        timer->stop();
    }
}
