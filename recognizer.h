#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSettings>
#include <QDirIterator>
#include <QDebug>
#include <QMessageBox>
#include <QProgressBar>
#include <QThread>
#include <qtconcurrentrun.h>
#include <QResizeEvent>


#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "detector.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace cv;

namespace Ui {
class Recognizer;
}

class Recognizer : public QMainWindow
{
    Q_OBJECT

signals:
    void TrainingObjects(QString type);

public:
    explicit Recognizer(QWidget *parent = 0);
    ~Recognizer();
    static Mat norm_0_255(InputArray _src);
    void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';');

    static const int CAMERATAB = 0;
    static const int CAMERAUPDATETIME = 100;
    static const int CLASSIFIERDURATION = 1000;

private slots:
    void onTrainingObjects(QString type);

    //*****************************Preparation*************************
    void on_CVS_Path_Button_clicked();
    void on_CVS_Path_lineEdit_textChanged(const QString &arg1);
    void on_CreateCVS_Button_clicked();

    void on_ClassifierLineEdit_textChanged(const QString &arg1);
    void on_ClassifierPath_toolButton_clicked();

    void on_LBPH_checkBox_clicked(bool checked);
    void on_LBPH_lineEdit_textChanged(const QString &arg1);
    void on_LBPH_toolButton_clicked();
    void on_EigenFace_checkBox_clicked(bool checked);
    void on_EigenFace_location_toolButton_clicked();
    void on_EigenFace_location_lineEdit_textChanged(const QString &arg1);
    void on_FisherFace_location_toolButton_clicked();
    void on_FisherFace_checkBox_clicked(bool checked);
    void on_FisherFace_location_lineEdit_textChanged(const QString &arg1);

    void on_SaveTrainedModel_pushButton_clicked();
    void on_LoadTrainedModel_pushButton_clicked();
    void on_Train_Button_clicked();
    //******************************************************************

    //*****************************Update*******************************
    void on_UpdateExisting_checkBox_clicked(bool checked);
    void on_UpdateLocation_lineEdit_textChanged(const QString &arg1);
    void on_UpdateLocation_toolButton_clicked();

    void on_TrainNew_pushButton_clicked();
    //******************************************************************

    //*****************************Prediction***************************
    void on_Target_lineEdit_textChanged(const QString &arg1);
    void on_Target_toolButton_clicked();
    void on_Predict_Button_clicked();
    //******************************************************************

    void on_CameraUpdateTime_valueChanged(int arg1);

    void on_ClassifierDuration_valueChanged(int arg1);

    void on_tabWidget_currentChanged(int index);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void Training();
    void Initialization();
    void SaveSettings();
    void LoadSettings();
    void AlgorithmChecked();

    Ui::Recognizer *settingsUI;
    QProgressBar *statProgress;

    QString CVS_Path;
    QString Output_Folder;
    QString Target_Path;

    QString ClassifierPath;
    Ptr<FaceRecognizer> LBPHModel;
    QString LBPHPath;
    Ptr<FaceRecognizer> EigenFaceModel;
    QString EigenFacePath;
    Ptr<FaceRecognizer> FisherFaceModel;
    QString FisherFacePath;

    int img_width;
    int img_height;

    QString UpdateExistingDirectory;

    Mat testSample;
    vector<Mat> images;
    vector<int> labels;
    Detector *detector;
};

#endif // RECOGNIZER_H
