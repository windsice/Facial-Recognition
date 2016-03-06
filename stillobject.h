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
#include <QLabel>

namespace Ui {
class StillObject;
}

class label_picture : public QLabel
{
protected:
    void mousePressEvent(QMouseEvent *event);
};


class StillObject : public QWidget, public StillDetection
{
    Q_OBJECT

signals:
    void doneDetection();

public:
    explicit StillObject(QWidget *parent = 0);
    ~StillObject();

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private slots:
    void on_toolButton_PictureSelection_clicked();

    void on_doubleSpinBox_ScaleFactor_valueChanged(double arg1);

    void on_spinBox_MinNeighbors_valueChanged(int arg1);

    void on_detectiondone();

    void on_checkBox_ColorCheck_clicked(bool checked);

private:
    void displayOnGUI();
    void updateFolderIterator();
    void startDetection();
    void performDetection();

    Ui::StillObject *ui;
    QString PicturePath;
    QDirIterator *folderIt = NULL;

    cv::Mat color;
};

#endif // STILLOBJECT_H
