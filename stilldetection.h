#ifndef STILLDETECTION_H
#define STILLDETECTION_H

#include <QString>
#include <QImage>
#include <QMessageBox>

#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

using namespace std;

class StillDetection{

public:

    virtual void displayOnGUI() = 0;

    // Perform detection and put rectangle around it.
    // save the result into Qimage, and ready to display by GUI.
    void performDetection(){
        objects.clear();
        haar_cascade.detectMultiScale(mat_picture_gray,objects);

        for(int i = 0; i < objects.size(); i++){
            cv.rectangle(mat_picture_original,objects[i],CV_RGB(0,255,0),1);
        }

        imgForDisplay = new QImage((uchar*)mat_picture_original.data,
                                   mat_picture_original.cols,
                                   mat_picture_original.rows,
                                   mat_picture_original.step,
                                   QImage::Format_RGB888);
    }

    // read image in regular and grayscale mode from the path
    inline void setImage(const QString &ImagePath){
        mat_picture_original = cv.imread(ImagePath.toStdString());
        cv.cvtColor(mat_picture_original,mat_picture_gray,CV_BGR2GRAY);
    }

    // this step is necessary for detection, which tells the machince how does the object looks like.
    inline void setClassifier(const QString &ClassifierPath){
        try{
            haar_cascade.load(ClassifierPath.toStdString());
        } catch (...) {
            QMessageBox::critical(this,"Classifier","Cannot load classifier");
        }
    }

protected:
    CascadeClassifier haar_cascade;     //holding the graphical characteristic of the object.
    vector< Rect_<int> > objects;       //holding the positions of detected objects.
    Mat mat_picture_original;           //Image that we want to perform detection on.
    Mat mat_picture_gray;               //A matrix that holds graymode of the picture is needed for detection
    QImage *imgForDisplay;

};


#endif // STILLDETECTION_H
