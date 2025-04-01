#include "FileMenu.h"
#include <QFileDialog>
#include <QDebug>

FileMenu::FileMenu(QWidget *parent) : QMenu("File", parent) {
    QAction *openAction = addAction("Open Files");
    connect(openAction, &QAction::triggered, this, &FileMenu::openFileDialog);
}

void FileMenu::openFileDialog() {
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Video Files",
        QString(),
        "Video Files (*.mp4 *.avi *.mkv *.flv *.mov *.wmv *.webm);;All Files (*.*)"
    );

    if (!files.isEmpty()) {
        emit filesSelected(files);
        //qDebug() << "Selected files:" << files;
    }
}
