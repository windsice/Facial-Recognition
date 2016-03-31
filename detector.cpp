#include "detector.h"
#include "ui_detector.h"
#include "QDebug"


Detector::Detector(QWidget *parent) : QWidget(parent), ui(new Ui::Detector)
{
    timer = new QTimer(this);
    clearFacesTimer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(ProcessFrame()));
    connect(clearFacesTimer,SIGNAL(timeout()),this,SLOT(ClearFaces()));
    ui->setupUi(this);

    layout = new QBoxLayout(QBoxLayout::TopToBottom,ui->Capturing_set_area);
    ui->Capturing_set_area->setLayout(layout);

    ui->label->setForegroundRole(QPalette::Light);
    ui->label_2->setForegroundRole(QPalette::Light);
    ui->Total_subject_label->setForegroundRole(QPalette::Light);
    ui->status->setForegroundRole(QPalette::Light);
    ui->checkBox_imageInGray->setStyleSheet("QCheckBox { color: white }");

    LoadSettings();
    totalSubjectCount();
    CameraStarted = false;
}

Detector::~Detector()
{
    if(CameraStarted){
        delete timer;
        delete camera;
        delete ui;
    }
}

void Detector::resizeEvent(QResizeEvent *event)
{
    event->accept();
    ScreenSize = this->size();
}

void Detector::setImageWidth(const int &w)
{
    img_width = w;
}

void Detector::setImageHeight(const int &h)
{
    img_height = h;
}

void Detector::setCameraResolution(const QSize &size){

    if(camera != NULL)
    {
        camera->set(CV_CAP_PROP_FRAME_WIDTH,size.width());
        camera->set(CV_CAP_PROP_FRAME_HEIGHT,size.height());
    }
}

void Detector::setClassifierDuration(const int &newtime)
{
    classifierDurationTime = newtime;
}

void Detector::setUpdatingTime(const int &newtime)
{
    updatingTime = newtime;
}

void Detector::setCameraDevice(const int &n)
{
    VideoCapture(n).release();
    camera = new VideoCapture(n);
    CameraStarted = true;
}

void Detector::setClassifier(const QString &path)
{
    haar_cascade.load(path.toStdString());
}

void Detector::setModels(bool aIsUsed, const Ptr<FaceRecognizer> &a, bool bIsUsed, const Ptr<FaceRecognizer> &b, bool cIsUsed,const Ptr<FaceRecognizer> &c)
{
    LBPHUsed = aIsUsed;
    LBPHModel = a;
    EigenFaceUsed = bIsUsed;
    EigenFaceModel = b;
    FisherFaceUsed = cIsUsed;
    FisherFaceModel = c;
}

bool Detector::Capturing()
{
    if(!camera->isOpened())
        return false;

    //set QUI thread to a lower priority, so that image will be processed more frequently
    QThread::currentThread()->setPriority(QThread::IdlePriority);
    startTimers();
    return true;
}

void Detector::Stop()
{
    if(!CameraStarted)
        return;

    if(camera->isOpened())
    {
        stopTimers();
        camera->release();

        //set back GUI thread priority.
        QThread::currentThread()->setPriority(QThread::NormalPriority);
    }
}

int Detector::getFrameWidth()
{
    return camera->get(CV_CAP_PROP_FRAME_WIDTH);
}

int Detector::getFrameHeight()
{
    return camera->get(CV_CAP_PROP_FRAME_HEIGHT);
}

int Detector::getFrameRate()
{
    return camera->get(CV_CAP_PROP_FPS);
}

void Detector::ProcessFrame()
{
    Mat frame;
    Mat img,img_resized;
    Captured_pic = new QLabel;
    camera->read(frame);
    gray = frame.clone();
    cvtColor(frame,gray,CV_BGR2GRAY);
    if(ui->checkBox_imageInGray->isChecked()){
        cvtColor(frame,frame,CV_BGR2GRAY);
        cvtColor(frame,frame,CV_GRAY2RGB);
    }
    else
        cvtColor(frame,frame,CV_BGR2RGB);

    if(!detectingThread.isRunning())
    detectingThread = QtConcurrent::run(this,&Detector::detectingFaces);

    for(unsigned int i=0;i < faces.size(); i++)
    {
        Rect face_i = faces[i];
        Mat face = gray(face_i);
        Mat face_resized;
        cv::resize(face,face_resized,Size(img_width,img_height),1.0,1.0,INTER_CUBIC);
        String box_text;
        if(LBPHUsed)
        {
            LBPHPrediction = LBPHModel->predict(face_resized);
            box_text = format("LBPH Prediction = %d ",LBPHPrediction);
        }
        if(EigenFaceUsed)
        {
            EigenFacePrediction = EigenFaceModel->predict(face_resized);
            box_text.append("EigenFace Prediction = %d ",EigenFacePrediction);
        }
        if(FisherFaceUsed)
        {
            FisherFacePrediction = FisherFaceModel->predict(face_resized);
            box_text.append("LBPHPrediction = %d ",FisherFacePrediction);
        }
        int pos_x = std::max(face_i.tl().x - 10,0);
        int pos_y = std::max(face_i.tl().y - 10,0);

        rectangle(frame,face_i,CV_RGB(0,255,0),1);
        putText(frame,box_text,Point(pos_x,pos_y),FONT_HERSHEY_PLAIN,1.0,CV_RGB(0,255,0),1.0);

        img = Mat(frame,face_i);
        cv::resize(img,img_resized,Size(img_width,img_height),1.0,1.0,INTER_CUBIC);

        Captured_frameImg = new QImage((uchar*)img_resized.data,img_resized.cols,img_resized.rows,img_resized.step,QImage::Format_RGB888);
        Captured_pic->setPixmap(QPixmap::fromImage(*Captured_frameImg));
        if(capturing_img)
            layout->addWidget(Captured_pic);
    }

    QImage frameImg((uchar*)frame.data,frame.cols,frame.rows,frame.step,QImage::Format_RGB888);
    ui->CameraScreen->setPixmap(QPixmap::fromImage(frameImg).scaled(ScreenSize,Qt::KeepAspectRatio));

    if(capturing_img)
        capturing_img = false;
}

void Detector::ClearFaces()
{
    faces.clear();
}

void Detector::detectingFaces()
{
    if(!timer->isActive())
        return;

    vector< Rect_<int> > temp_faces;
    haar_cascade.detectMultiScale(gray,temp_faces);
    if(temp_faces.size()>0)
        faces = temp_faces;
}

void Detector::totalSubjectCount()
{
    try{
        if(QDir(RootPath).exists() && RootPath != "")
        {
            int count = 0;
            QDirIterator Folders(RootPath,QDir::AllDirs | QDir::NoDot | QDir::NoDotDot);
            while(Folders.hasNext())
            {
                count++;
                Folders.next();
            }

            ui->Total_subject_label->setText("Total Subjects: " + QString::number(count));
        }
    } catch(Exception e)
    {
        QMessageBox::critical(this,"Error",QString::fromStdString(e.msg));
    }
}

void Detector::SaveSettings()
{
    QSettings settings("Settings.ini",QSettings::IniFormat);
    settings.beginGroup("FileLocation");
    settings.setValue("Root Path",RootPath);
    settings.endGroup();
}

void Detector::LoadSettings()
{
    QSettings settings("Settings.ini",QSettings::IniFormat);
    settings.beginGroup("FileLocation");
    ui->RootPath_lineEdit->setText(settings.value("Root Path").toString());
    settings.endGroup();
}


void Detector::on_CaptureNewObject_clicked()
{
    if(img_width == 0 || img_height == 0)
    {
        QMessageBox::information(this,"Information Missing","Please train the model first");
        return;
    }
    capturing_img = true;
    delete layout;
    layout = new QBoxLayout(QBoxLayout::TopToBottom,ui->Capturing_set_area);

}

void Detector::on_AddToLabel_Button_clicked()
{
    QString tempPath = RootPath;
    if(!RootPath.isNull() || !RootPath.isEmpty())
    {
        //get label number and append it to the path, thus become the path of the label
        tempPath.append("/s");
        tempPath.append(QString::number(ui->AddToLabel_spinBox->value()));

        if(!QDir(tempPath).exists())
            QDir().mkdir(tempPath);

        //get the last number from the set
        int count = 1;
        QDirIterator Files(tempPath,QDir::Files | QDir::NoDot | QDir::NoDotDot);
        while(Files.hasNext())
        {
            count++;
            Files.next();
        }
        tempPath.append("/");
        tempPath.append(QString::number(count));
        tempPath.append(".jpg");

        try{
            Captured_pic->pixmap()->save(tempPath);
            tempPath.append(" Saved");
            ui->status->setText(tempPath);
        } catch (Exception e)
        {
            QMessageBox::critical(this,"Error","Did you capture yet?");
        }

        totalSubjectCount();
    }
}

void Detector::on_RootPath_lineEdit_textChanged(const QString &arg1)
{
    RootPath = arg1;
}

void Detector::on_RootPath_toolButton_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getExistingDirectory(this,"Picture Database Directory",RootPath,QFileDialog::ShowDirsOnly);
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        ui->RootPath_lineEdit->setText(tempPath);
        RootPath = tempPath;
        totalSubjectCount();
    }
    SaveSettings();
}

void Detector::startTimers()
{
    timer->start(this->updatingTime);
    clearFacesTimer->start(this->classifierDurationTime);
}

void Detector::stopTimers()
{
    timer->stop();
    clearFacesTimer->stop();
}
