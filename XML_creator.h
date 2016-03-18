#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QEvent>
#include <QDirIterator>
#include <QResizeEvent>
#include <QFileInfo>
#include <QtGui>
#include <QtCore>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_negpath_clicked();

    void on_pushButton_pospath_clicked();

    void on_lineEdit_targetname_textChanged(const QString);

    void on_pushButton_sample_clicked();

    void on_pushButton_XML_clicked();

    void getProcessOutput();

    void generateFileXML(const int &result);

private:
    Ui::MainWindow *ui;
    QString targetname;
    QImage posdisplay;
    QPixmap posdisplay_px;
    QString positiveimgpath;
    void get_negative_image_list();
    void write();

    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    QDirIterator *pictureIt = NULL;
    void updatePictureIterator();
    void displayPositiveImage();
    QPoint getPixmapTopleftPos();
    void createPositiveImageVector();
    void haarTraining();
    void resetCascadeFolder();



    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    QPoint qpPixmapInitial,qpPixmapFinal,qpPixmapDimension,qpMainInitial,qpMainFinal;
    int positiveImageLoaded;//avoid crashes
    double pictureRatio;
    QString PositiveInfo;
    int NumberOfSelectedObject;
    int NumberOfPositiveImage;
    int NumberOfPositiveImageDisplayed;
    QString positiveInfoFileName;
    bool mouseIsOnPixmap;
    bool displayRect;
    int processedImageNumber;
    bool stopAllOtherFunction;

QProcess *haarTrain;



protected:
    void paintEvent(QPaintEvent *p);

};

#endif // MAINWINDOW_H
