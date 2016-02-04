#ifndef STILLOBJECT_H
#define STILLOBJECT_H

#include <QWidget>
#include "stilldetection.h"
#include <QFileDialog>
#include <QString>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>

namespace Ui {
class StillObject;
}

class StillObject : public QWidget, public StillDetection
{
    Q_OBJECT

public:
    explicit StillObject(QWidget *parent = 0);
    ~StillObject();

protected:
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

private slots:
    void on_toolButton_PictureSelection_clicked();

private:
    void displayOnGUI();
    void updateFolderIterator();

    Ui::StillObject *ui;
    QString PicturePath;
    QDirIterator *folderIt = NULL;
};

#endif // STILLOBJECT_H
