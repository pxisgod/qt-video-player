#include <QApplication>
#include <QWidget>
#include "MainWindow.h"
 
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
 
    //QWidget window;
    //window.resize(250, 150); // 设置窗口大小
    MainWindow window;
    window.setWindowTitle("Video Player"); // 设置窗口标题
    window.show(); // 显示窗口


    return app.exec();
}