#include "stillobject.h"
#include "ui_stillobject.h"

StillObject::StillObject(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StillObject)
{
    ui->setupUi(this);
    scaleFactor = ui->doubleSpinBox_ScaleFactor->value();
    minNeighbors = ui->spinBox_MinNeighbors->value();
    on_checkBox_ColorCheck_clicked(false);

    connect(this,SIGNAL(doneDetection()),this,SLOT(on_detectiondone()));
}

StillObject::~StillObject()
{
    delete ui;
}

void StillObject::displayOnGUI(){
    if(PicturePath.isNull() || PicturePath.isEmpty())
        return;
    setImage(PicturePath);
    startDetection();
    QSize size = ui->label_StillObject->size();
    ui->label_StillObject->setPixmap(QPixmap(PicturePath).scaled(size,Qt::KeepAspectRatio));
}

void StillObject::updateFolderIterator(){

    if(folderIt != NULL){
        delete folderIt;
        folderIt = NULL;
    }
    folderIt = new QDirIterator(QFileInfo(PicturePath).absolutePath(),QDir::Files);
}

void StillObject::resizeEvent(QResizeEvent *event){

    if(PicturePath.isEmpty()){
        event->ignore();
    } else {
        QSize size = ui->label_StillObject->size();
        ui->label_StillObject->setPixmap(QPixmap(PicturePath).scaled(size,Qt::KeepAspectRatio));
        event->accept();
    }
}

void StillObject::mouseDoubleClickEvent(QMouseEvent *event){
    if(folderIt == NULL || performingThread.isRunning()){
        event->ignore();
        return;
    }

    //loop around
    if(!folderIt->hasNext()){
        folderIt = new QDirIterator(QFileInfo(PicturePath).absolutePath(),QDir::Files);
    }

    if(folderIt->hasNext()){
        event->accept();
        PicturePath = folderIt->next();
        displayOnGUI();
    }

}

void StillObject::on_toolButton_PictureSelection_clicked()
{
    QString tempPath = QFileDialog::getOpenFileName(this,"Select picture",PicturePath,"Images (*.png *.bmp *jpg *.pgm)");
    if(QFileDialog::Accepted && !tempPath.isNull()){
        PicturePath = tempPath;
        displayOnGUI();
        updateFolderIterator();
    }
}

void StillObject::on_doubleSpinBox_ScaleFactor_valueChanged(double arg1)
{
    setScaleFactor(arg1);
    displayOnGUI();
}

void StillObject::on_spinBox_MinNeighbors_valueChanged(int arg1)
{
    setMinNeighbors(arg1);
    displayOnGUI();
}


void StillObject::on_detectiondone(){

    QSize size = ui->label_StillObject->size();
    ui->label_StillObject->setPixmap(QPixmap::fromImage(*imgForDisplay).scaled(size,Qt::KeepAspectRatio));
    ui->label_ColorRange->setPixmap(QPixmap::fromImage(*imgForColorRange).scaled(size,Qt::KeepAspectRatio));
    if(objects.size() > 0)
        ui->label_ObjectCount->setText(QString::number(objects.size())+" objects");
    else
        ui->label_ObjectCount->setText("");
}

void StillObject::startDetection(){
    performingThread = QtConcurrent::run(this,&StillObject::performDetection);
    ui->label_ObjectCount->setText("Loading...");
}

// Perform detection and put rectangle around it.
// save the result into Qimage, and ready to display by GUI.
void StillObject::performDetection(){
    objects.clear();
    haar_cascade.detectMultiScale(mat_picture_gray,objects,scaleFactor,minNeighbors);

    for(unsigned int i = 0; i < objects.size(); i++){
        cv::rectangle(mat_picture_original,objects[i],CV_RGB(0,255,0),1);
    }

    cv::inRange(mat_picture_original,
                cv::Scalar(ui->spinBox_LBH->value(),ui->spinBox_LBS->value(),
                           ui->spinBox_LBV->value()),
                cv::Scalar(ui->spinBox_UBH->value(),ui->spinBox_UBS->value(),
                           ui->spinBox_UBV->value()),color);
    cv::cvtColor(color,color,CV_GRAY2RGB);
    cv::cvtColor(mat_picture_original,mat_picture_original,CV_BGR2RGB);

    imgForDisplay = new QImage((uchar*)mat_picture_original.data,
                               mat_picture_original.cols,
                               mat_picture_original.rows,
                               mat_picture_original.step,
                               QImage::Format_RGB888);

    imgForColorRange = new QImage((uchar*)color.data,
                                  color.cols,
                                  color.rows,
                                  color.step,
                                  QImage::Format_RGB888);


    emit StillObject::doneDetection();
}

void StillObject::on_checkBox_ColorCheck_clicked(bool checked)
{
    if(checked){
        ui->groupBox_ColorRange->setVisible(true);
    } else {
        ui->groupBox_ColorRange->setVisible(false);
    }
    displayOnGUI();
}

