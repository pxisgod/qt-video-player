#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include "VideoPlayerWidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

private:
    VideoPlayerWidget *playerWidget;
    void setupUI();
};

#endif // MAINWINDOW_H

