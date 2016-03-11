#ifndef STILLDETECTION_H
#define STILLDETECTION_H

#include <QString>
#include <QImage>
#include <QFuture>
#include <QtConcurrent>

#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

using namespace std;

class StillDetection {


public:

    virtual void displayOnGUI() = 0;

    inline void setScaleFactor(const double &sf){
        scaleFactor = sf;
    }
    inline void setMinNeighbors(const int &mN){
        minNeighbors = mN;
    }

    // read image in regular and grayscale mode from the path
    inline void setImage(const QString &ImagePath){
        imgOriginal.load(ImagePath);
        mat_picture_original = cv::imread(ImagePath.toStdString());
        cv::cvtColor(mat_picture_original,mat_picture_gray,CV_BGR2GRAY);
        cv::cvtColor(mat_picture_original,mat_picture_rgb,CV_BGR2RGB);
        cv::cvtColor(mat_picture_original,mat_picture_hsv,CV_BGR2HSV);
        mat_picture_empty.create(mat_picture_original.rows,mat_picture_original.cols,mat_picture_gray.depth());
    }

    // this step is necessary for detection, which tells the machince how does the object looks like.
    inline bool setClassifier(const QString &ClassifierPath){
        try{
            haar_cascade.load(ClassifierPath.toStdString());
            return true;
        } catch (...) {
            return false;
        }
    }

    cv::Vec3b ConvertColor( cv::Vec3b src, int code)
    {
        cv::Mat srcMat(1, 1, CV_8UC3 );
        *srcMat.ptr< cv::Vec3b >( 0 ) = src;

        cv::Mat resMat;
        cv::cvtColor( srcMat, resMat, code);

        return *resMat.ptr< cv::Vec3b >( 0 );
    }

protected:
    cv::CascadeClassifier haar_cascade;     //holding the graphical characteristic of the object.
    vector< cv::Rect_<int> > objects;       //holding the positions of detected objects.
    cv::Mat mat_colorPick_object;           //holding the object we want to perfer color detection.
    cv::Mat mat_picture_empty;              //holding the picture size information, but without image.
    cv::Mat mat_picture_original;           //Image that we want to perform detection on.
    cv::Mat mat_picture_hsv;                //Image that we want to perform color detection.
    cv::Mat mat_picture_rgb;                //Image that we want to display.
    cv::Mat mat_picture_gray;               //A matrix that holds graymode of the picture is needed for detection.
    QPixmap imgOriginal;
    QImage *imgForDisplay = NULL;
    QImage *imgForColorRange = NULL;
    double scaleFactor;
    int minNeighbors;
    QFuture<void> performingThread;
};


#endif // STILLDETECTION_H
