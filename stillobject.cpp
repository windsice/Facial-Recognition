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
    ui->label_StillObject->setPixmap(imgOriginal.scaled(size,Qt::KeepAspectRatio));
}

void StillObject::initFolderIterator(){

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
        resizeProcessedPictures();
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

void StillObject::mousePressEvent(QMouseEvent *event)
{
    if(!ui->checkBox_ColorCheck->isChecked()){
        event->ignore();
        return;
    }

    //get coordinates in the picture
    QPoint p = ui->label_StillObject->mapFromParent(event->pos());
    QSize Correction = (ui->label_StillObject->size() - ui->label_StillObject->pixmap()->size())/2;
    int displayW = ui->label_StillObject->pixmap()->width();
    int displayH = ui->label_StillObject->pixmap()->height();
    QPoint CorrectedP(p.x() - Correction.width(),p.y() - Correction.height());
    if(CorrectedP.x() < 0 || CorrectedP.y() < 0 ||
            CorrectedP.x() > displayW || CorrectedP.y() > displayH)
    {
        event->ignore();
        return;
    }

    //pass the coordinate to opencv to process the color
    QPoint ResizedP((float)CorrectedP.x()/(float)displayW*imgOriginal.width(),
                    (float)CorrectedP.y()/(float)displayH*imgOriginal.height());
    cv::Vec3b pixelColor = mat_picture_original.at<cv::Vec3b>(cv::Point(ResizedP.x(),ResizedP.y()));
    // red = pixelColor.val[2]
    // greeb = pixelColor.val[1]
    // blue = pixelColor.val[0]

    pixelColorHSV = ConvertColor(pixelColor,CV_BGR2HSV);
    // H = pixelColor.val[0]
    // S = pixelColor.val[1]
    // V = pixelColor.val[2]

    startDetection();
}

void StillObject::on_toolButton_PictureSelection_clicked()
{
    QString tempPath = QFileDialog::getOpenFileName(this,"Select picture",PicturePath,"Images (*.png *.bmp *jpg *.pgm)");
    if(QFileDialog::Accepted && !tempPath.isNull()){
        PicturePath = tempPath;
        displayOnGUI();
        initFolderIterator();
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
    resizeProcessedPictures();
    QString text;
    if(objects.size() > 0)
        text.append(QString::number(objects.size())+" objects");
    else if(ui->checkBox_ColorCheck->isChecked())
        text.append("\tH:" + QString::number(pixelColorHSV.val[0]) +
                    " S:" + QString::number(pixelColorHSV.val[1]) +
                    " V:" + QString::number(pixelColorHSV.val[2]));
    else
        text.clear();
    ui->label_ObjectCount->setText(text);
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
        cv::rectangle(mat_picture_rgb,objects[i],CV_RGB(0,255,0),1);
    }

    imgForDisplay = new QImage((uchar*)mat_picture_rgb.data,
                               mat_picture_rgb.cols,
                               mat_picture_rgb.rows,
                               mat_picture_rgb.step,
                               QImage::Format_RGB888);

    colorPicker();

    emit StillObject::doneDetection();
}

void StillObject::colorPicker()
{
    int lowerResult[3] = { pixelColorHSV.val[0] - ui->spinBox_LBH->value(),
                           ColorBound(pixelColorHSV.val[1] - ui->spinBox_LBS->value()),
                           ColorBound(pixelColorHSV.val[2] - ui->spinBox_LBV->value())};
    int upperResult[3] = { pixelColorHSV.val[0] + ui->spinBox_UBH->value(),
                           ColorBound(pixelColorHSV.val[1] + ui->spinBox_UBS->value()),
                           ColorBound(pixelColorHSV.val[2] + ui->spinBox_UBV->value())};

    //wrap around needed for hue value, since 179 and 1 both represent red
    if(lowerResult[0] < 0){
        cv::Mat wrap;
        cv::Mat limit;
        //wrap up part
        cv::inRange(mat_picture_original,
                    cv::Scalar( 180 + lowerResult[0],lowerResult[1],lowerResult[2]),
                    cv::Scalar( 180,upperResult[1],upperResult[2]),
                    wrap);

        //limited part
        cv::inRange(mat_picture_original,
                    cv::Scalar( 0,lowerResult[1],lowerResult[2]),
                    cv::Scalar( upperResult[0],upperResult[1],upperResult[2]),
                    limit);

        color = wrap + limit;

        qDebug() << "lowerbound: " << QString::number(180 + lowerResult[0]) << " " << QString::number(upperResult[0]);

    } else if (upperResult[0] > 180){
        cv::Mat wrap;
        cv::Mat limit;
        //wrap up part
        cv::inRange(mat_picture_original,
                    cv::Scalar( 0,lowerResult[1],lowerResult[2]),
                    cv::Scalar( upperResult[0] - 180,upperResult[1],upperResult[2]),
                    wrap);

        //limited part
        cv::inRange(mat_picture_original,
                    cv::Scalar( lowerResult[0],lowerResult[1],lowerResult[2]),
                    cv::Scalar( 180,upperResult[1],upperResult[2]),
                    limit);
        color = wrap + limit;

        qDebug() << "upperbound: " << QString::number(upperResult[0] - 180) << " " << QString::number(lowerResult[0]);
    } else {

        cv::inRange(mat_picture_original,
                    cv::Scalar(lowerResult[0],lowerResult[1],lowerResult[2]),
                    cv::Scalar(upperResult[0],upperResult[1],upperResult[2]),
                    color);
    }

    cv::cvtColor(color,color,CV_GRAY2RGB);

    imgForColorRange = new QImage((uchar*)color.data,
                                  color.cols,
                                  color.rows,
                                  color.step,
                                  QImage::Format_RGB888);
}

void StillObject::resizeProcessedPictures()
{
    QSize size = ui->label_StillObject->size();
    ui->label_StillObject->setPixmap(QPixmap::fromImage(*imgForDisplay).scaled(size,Qt::KeepAspectRatio));
    ui->label_ColorRange->setPixmap(QPixmap::fromImage(*imgForColorRange).scaled(size,Qt::KeepAspectRatio));
}

//Check if the value is within 0 and 255, if not return the bound.
int StillObject::ColorBound(const int color)
{
    if(color > 255)
        return 255;
    if(color < 0)
        return 0;

    return color;
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

