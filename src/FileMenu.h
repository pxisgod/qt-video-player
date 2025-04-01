#ifndef FILEMENU_H
#define FILEMENU_H

#include <QMenu>
#include <QStringList>

class FileMenu : public QMenu {
    Q_OBJECT

public:
    explicit FileMenu(QWidget *parent = nullptr);

signals:
    void filesSelected(const QStringList &files);

private slots:
    void openFileDialog();
};

#endif // FILEMENU_H
