#ifndef PROGRESS_WIDGET_H
#define PROGRESS_WIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QTimer>

class ProgressWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProgressWidget(QWidget *parent = nullptr);

signals:
    void seek(int position);

public slots:
    void start_timer();
    void stop_timer();
    void set_duration(int duration_in_seconds);

private:
    QSlider *progress_bar;
    QLabel *time_label;
    QTimer *timer;

private slots:
    void update_progress();
    void show_tooltip(int position);
    void hide_tooltip();
};

#endif // PROGRESS_WIDGET_H
