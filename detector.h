#ifndef DETECTOR_H
#define DETECTOR_H

#include <QWidget>
#include <QFileDialog>
#include <QDirIterator>
#include <QSettings>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QLabel>
#include <qtconcurrentrun.h>
#include <QBoxLayout>
#include <QResizeEvent>

#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

using namespace std;
using namespace cv;

namespace Ui {
class Detector;
}

class Detector : public QWidget
{
    Q_OBJECT

public:
    explicit Detector(QWidget *parent = 0);
    ~Detector();

protected:
    void resizeEvent(QResizeEvent *event);

public:
    void setColorBound(int lowerH, int lowerS, int lowerV, int upperH, int upperS, int upperV);
    void setImageWidth(const int&);
    void setImageHeight(const int&);
    void setCameraResolution(const QSize&);

    void setClassifierDuration(const int &);
    void setUpdatingTime(const int&);
    void setCameraDevice(const int &);
    void setClassifier(const QString &);
    void setColorClassifier(const QString &path);
    void setModels(bool aIsUsed, const Ptr<FaceRecognizer> &, bool bIsUsed, const Ptr<FaceRecognizer> &, bool cIsUsed, const Ptr<FaceRecognizer> &);
    bool Capturing();
    void Stop();

    int getFrameWidth();
    int getFrameHeight();
    int getFrameRate();

private slots:
    void ProcessFrame();
    void ClearFaces();

    void on_CaptureNewObject_clicked();

    void on_AddToLabel_Button_clicked();

    void on_RootPath_lineEdit_textChanged(const QString &arg1);

    void on_RootPath_toolButton_clicked();

private:
    void startTimers();
    void stopTimers();
    void detectingFaces();
    void detectingColorFaces();
    void totalSubjectCount();
    void SaveSettings();
    void LoadSettings();

    VideoCapture *camera;
    Ui::Detector *ui;

    bool LBPHUsed;
    Ptr<FaceRecognizer> LBPHModel;
    int LBPHPrediction;
    bool EigenFaceUsed;
    Ptr<FaceRecognizer> EigenFaceModel;
    int EigenFacePrediction;
    bool FisherFaceUsed;
    Ptr<FaceRecognizer> FisherFaceModel;
    int FisherFacePrediction;

    int img_width;
    int img_height;

    QImage *Captured_frameImg;
    QString RootPath;
    bool capturing_img;
    bool detecting;
    QLabel *Captured_pic;
    QBoxLayout *layout;
    CascadeClassifier haar_cascade;
    CascadeClassifier Color_haar_cascade;

    int Color_lowerBound[3];
    int Color_upperBound[3];

    vector< Rect_<int> > faces;
    vector< Rect_<int> > Color_faces;

    QFuture<void> detectingThread;
    QFuture<void> detectingColorThread;
    int updatingTime;
    int classifierDurationTime;
    QSize ScreenSize;
    QTimer *timer;
    QTimer *clearFacesTimer;

    Mat frame;
    Mat gray;
    Mat Color_gray;

    bool CameraStarted; //unable to find a way to check if camera is null, thus use this flag.
};

#endif // DETECTOR_H


