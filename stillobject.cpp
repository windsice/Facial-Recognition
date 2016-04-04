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

    // skip iterator so that the next one won't always be the same
    while(folderIt->filePath() != PicturePath)
        folderIt->next();
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

    QPoint ResizedP;
    if(!ui->checkBox_lockColor->isChecked()){
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
        ResizedP.setX((float)CorrectedP.x()/(float)displayW*imgOriginal.width());
        ResizedP.setY((float)CorrectedP.y()/(float)displayH*imgOriginal.height());
        cv::Vec3b pixelColor = mat_picture_original.at<cv::Vec3b>(cv::Point(ResizedP.x(),ResizedP.y()));
        // red = pixelColor.val[2]
        // greeb = pixelColor.val[1]
        // blue = pixelColor.val[0]

        pixelColorHSV = ConvertColor(pixelColor,CV_BGR2HSV);
        // H = pixelColor.val[0]
        // S = pixelColor.val[1]
        // V = pixelColor.val[2]
    } else {
        pixelColorHSV[0] = ui->spinBox_H->value();
        pixelColorHSV[1] = ui->spinBox_S->value();
        pixelColorHSV[2] = ui->spinBox_V->value();
    }

    mat_colorPick_object.release();
    cv::Point p(ResizedP.x(),ResizedP.y());
    for(unsigned int i = 0; i < objects.size(); i++){
        if(objects[i].contains(p)){
            mat_colorPick_object = mat_picture_hsv(objects[i]);
            break;
        }
    }

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
    else
        text.clear();

    if(ui->checkBox_ColorCheck->isChecked()){
        ui->spinBox_H->setValue(pixelColorHSV.val[0]);
        ui->spinBox_S->setValue(pixelColorHSV.val[1]);
        ui->spinBox_V->setValue(pixelColorHSV.val[2]);
    }
    ui->label_ObjectCount->setText(text);
}

void StillObject::startDetection(){
    ui->label_FilePath->setText("Current File: " + PicturePath);
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

    if(ui->checkBox_ColorCheck->isChecked()){
        if(mat_colorPick_object.empty())
            colorPicker(mat_picture_hsv);
        else
            colorPicker(mat_colorPick_object);
    }

    emit StillObject::doneDetection();
}

void StillObject::updateColorBound()
{
    LowerBound_color[0] = pixelColorHSV.val[0] - ui->spinBox_LBH->value();
    LowerBound_color[1] = ColorBound(pixelColorHSV.val[1] - ui->spinBox_LBS->value());
    LowerBound_color[2] = ColorBound(pixelColorHSV.val[2] - ui->spinBox_LBV->value());
    UpperBound_color[0] = pixelColorHSV.val[0] + ui->spinBox_UBH->value();
    UpperBound_color[1] = ColorBound(pixelColorHSV.val[1] + ui->spinBox_UBS->value());
    UpperBound_color[2] = ColorBound(pixelColorHSV.val[2] + ui->spinBox_UBV->value());
}

QImage StillObject::colorPicker(cv::Mat image)
{
    updateColorBound();

    //wrap around needed for hue value, since 179 and 1 both represent red
    if(LowerBound_color[0] < 0){
        cv::Mat wrap;
        cv::Mat limit;
        //wrap up part
        cv::inRange(image,
                    cv::Scalar( 180 + LowerBound_color[0],LowerBound_color[1],LowerBound_color[2]),
                    cv::Scalar( 180,UpperBound_color[1],UpperBound_color[2]),
                    wrap);

        //limited part
        cv::inRange(image,
                    cv::Scalar( 0,LowerBound_color[1],LowerBound_color[2]),
                    cv::Scalar( UpperBound_color[0],UpperBound_color[1],UpperBound_color[2]),
                    limit);

        mat_colorResult = wrap + limit;

    } else if (UpperBound_color[0] > 180){
        cv::Mat wrap;
        cv::Mat limit;
        //wrap up part
        cv::inRange(image,
                    cv::Scalar( 0,LowerBound_color[1],LowerBound_color[2]),
                    cv::Scalar( UpperBound_color[0] - 180,UpperBound_color[1],UpperBound_color[2]),
                    wrap);

        //limited part
        cv::inRange(image,
                    cv::Scalar( LowerBound_color[0],LowerBound_color[1],LowerBound_color[2]),
                    cv::Scalar( 180,UpperBound_color[1],UpperBound_color[2]),
                    limit);
        mat_colorResult = wrap + limit;

    } else {
        cv::inRange(image,
                    cv::Scalar(LowerBound_color[0],LowerBound_color[1],LowerBound_color[2]),
                    cv::Scalar(UpperBound_color[0],UpperBound_color[1],UpperBound_color[2]),
                    mat_colorResult);
    }

    cv::cvtColor(mat_colorResult,mat_colorResult,CV_GRAY2RGB);

    imgForColorRange = new QImage((uchar*)mat_colorResult.data,
                                  mat_colorResult.cols,
                                  mat_colorResult.rows,
                                  mat_colorResult.step,
                                  QImage::Format_RGB888);
    return *imgForColorRange;
}

void StillObject::resizeProcessedPictures()
{
    QSize size = ui->label_StillObject->size();
    ui->label_StillObject->setPixmap(QPixmap::fromImage(*imgForDisplay).scaled(size,Qt::KeepAspectRatio));
    if(ui->checkBox_ColorCheck->isChecked())
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
        ui->pushButton_outputResults->setVisible(true);
        ui->label_ColorRange->setVisible(true);
    } else {
        ui->groupBox_ColorRange->setVisible(false);
        ui->pushButton_outputResults->setVisible(false);
        ui->label_ColorRange->setVisible(false);
    }
    displayOnGUI();
}


void StillObject::on_pushButton_outputResults_clicked()
{
    QString outputFolder;
    outputFolder = QFileDialog::getExistingDirectory(this,"Output location","");
    if(outputFolder.isNull() || outputFolder.isEmpty())
        QMessageBox::critical(this,"error","Check your path");

    //Write a README file
    QString readme_content;
    readme_content += "Lower Bound: ";
    for(int i = 0 ; i < 3 ; i++){
        readme_content += QString::number(LowerBound_color[i]) + " ";
    }
    readme_content += "\nUpper Bound: ";
    for(int i = 0 ; i < 3 ; i++){
        readme_content += QString::number(UpperBound_color[i]) + " ";
    }

    QFile file(outputFolder + "/README.txt");
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        stream << readme_content << endl;
    }

    QDirIterator it(QFileInfo(PicturePath).absolutePath(),QDir::Files);
    QString outputPath;
    while(it.hasNext()){
        it.next();

        outputPath = QString("%1%2").arg(outputFolder + "/").arg(it.fileName());
        ui->label_ObjectCount->setText("Working on: " + outputPath);

        cv::Mat imageMat = cv::imread(it.filePath().toStdString());
        cv::cvtColor(imageMat,imageMat,CV_BGR2HSV);
        QImage image = colorPicker(imageMat);
        image.save(outputPath);
    }
    ui->label_ObjectCount->setText("");

    QMessageBox::information(this,"Output to folder","done");

}
