#ifndef XML_CREATOR_H
#define XML_CREATOR_H

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
class XML_creator;
}

class XML_creator : public QWidget
{
    Q_OBJECT

public:
    explicit XML_creator(QWidget *parent = 0);
    ~XML_creator();

private slots:
    void on_pushButton_negpath_clicked();

    void on_pushButton_pospath_clicked();

    void on_lineEdit_targetname_textChanged(const QString);

    void on_pushButton_sample_clicked();

    void on_pushButton_XML_clicked();

    void getProcessOutput();

    void generateFileXML(const int &result);

private:
    Ui::XML_creator *ui;
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

#endif // XML_CREATOR_H
