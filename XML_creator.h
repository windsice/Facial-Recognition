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
#include <QStatusBar>
#include <QMessageBox>

namespace Ui {
class XML_creator;
}

class XML_creator : public QWidget
{
    Q_OBJECT

public:

    const static QString CASCADETRAININGFOLDER;

    explicit XML_creator(QWidget *parent = 0);
    ~XML_creator();

    inline void SetPositivePath(const QString &path){
        positiveFolderPath = path;
        positiveImageLoaded = true;
    }
    inline void SetNegativePath(const QString &path){
        negativeFolderPath = path;
        get_negative_image_list();
    }
    inline void SetObjectName(const QString &name){
        targetName = name;
    }
    inline void SetObjectPath(const QDir &dir){
        targetPath = dir;
    }

    void get_negative_image_list();

private slots:
    void on_pushButton_sample_clicked();

    void on_pushButton_XML_clicked();

    void getProcessOutput();

    void generateFileXML(const int &result);

private:
    Ui::XML_creator *ui;
    QStatusBar *statusBar;

    QString targetName;
    QDir targetPath;
    QImage posdisplay;
    QPixmap posdisplay_px;
    QString positiveimgpath;
    QString positiveFolderPath;
    QString negativeFolderPath;
    QString vecPath;
    QString tempCascadePath;
    void write();

    QDirIterator *pictureIt = NULL;
    void updatePictureIterator();
    void displayPositiveImage();
    QPoint getPixmapTopleftPos();
    void createPositiveImageVector();
    void haarTraining();
    void resetCascadeFolder();

    QPoint qpPixmapInitial,qpPixmapFinal,qpPixmapDimension,qpMainInitial,qpMainFinal;
    int positiveImageLoaded;//avoid crashes
    double pictureRatio;
    QString PositiveInfo;
    int NumberOfSelectedObject;
    int NumberOfPositiveImage;
    int NumberOfPositiveImageDisplayed;
    int numberofn;
    QString positiveInfoFileName;
    QString negativeInfoFileName;
    bool mouseIsOnPixmap;
    bool displayRect;
    bool stopAllOtherFunction;

QProcess *haarTrain;

protected:
    void paintEvent(QPaintEvent *p);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

};

#endif // XML_CREATOR_H
