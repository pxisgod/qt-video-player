#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMediaPlaylist>
#include <QMenuBar>
#include <QMenu>
#include <QLabel>
#include "FileMenu.h"
#include "ProgressWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), playerWidget(new VideoPlayerWidget(this)) {

    // 创建菜单栏
    QMenuBar *menuBar = new QMenuBar(this);
    FileMenu *fileMenu = new FileMenu(menuBar);
    menuBar->addMenu(fileMenu);
    setMenuBar(menuBar);

    // 添加菜单项
    QAction *exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);

    // 关联 filesSelected 信号到 lambda 表达式
    connect(fileMenu, &FileMenu::filesSelected, this, [this](const QStringList &files) {
        for (const QString &file : files) {
            playerWidget->add_to_play_list(file); 
        }
    });

    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {

    QWidget *mainWindow = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWindow);
    mainLayout->addWidget(playerWidget);

    // 添加欢迎标签
    QLabel *label = new QLabel("Welcome to Video Player!", playerWidget);
    mainLayout->addWidget(label);

    setCentralWidget(mainWindow);
}