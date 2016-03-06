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
        mat_picture_original = cv::imread(ImagePath.toStdString());
        cv::cvtColor(mat_picture_original,mat_picture_gray,CV_BGR2GRAY);
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



protected:
    cv::CascadeClassifier haar_cascade;     //holding the graphical characteristic of the object.
    vector< cv::Rect_<int> > objects;       //holding the positions of detected objects.
    cv::Mat mat_picture_original;           //Image that we want to perform detection on.
    cv::Mat mat_picture_gray;               //A matrix that holds graymode of the picture is needed for detection.
    QImage *imgForDisplay = NULL;
    QImage *imgForColorRange = NULL;
    double scaleFactor;
    int minNeighbors;
    QFuture<void> performingThread;
};


#endif // STILLDETECTION_H
