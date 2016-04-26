#include "XML_creator.h"
#include "ui_XML_creator.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QKeyEvent>
#include <QPixmap>
#include <QWidget>
#include <QStringList>

const QString XML_creator::CASCADETRAININGFOLDER = QDir::currentPath() + "/cascade_training";

XML_creator::XML_creator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::XML_creator)
{
    ui->setupUi(this);

    positiveImageLoaded = false;
    stopAllOtherFunction = false;

    statusBar = new QStatusBar(this);
    ui->statusBarLayout->addWidget(statusBar);
}

XML_creator::~XML_creator()
{
    delete ui;
}

//display negative image list and write the list to a ****bg.txt file
void XML_creator::get_negative_image_list(){
    QDirIterator it(negativeFolderPath,QDir::Files);
    numberofn = 0;
    negativeInfoFileName = QString("%1/%2").arg(targetPath.absolutePath()).arg("neg.txt");

    QFile file;
    file.setFileName(negativeInfoFileName);
    if (!file.open(QIODevice::Append | QIODevice::Text)){
        qDebug()<<"could not open file";
    }
    QTextStream out(&file);
    while(it.hasNext()){

        it.next();
        out  <<it.filePath()<<'\n';
        numberofn++;

        ui->plainTextEdit->appendPlainText(it.fileName());
    }
    file.close();

    QString NegativeResult = QString("%1%2").arg( numberofn).arg(" negative image are provided");
    ui->plainTextEdit->appendPlainText(NegativeResult);
    ui->plainTextEdit->appendPlainText("-----------Proccess Completed-----------");
}

//start creating sample process
void XML_creator::on_pushButton_sample_clicked(){
    int processedImageNumber = 0;
    NumberOfPositiveImage = 0;

    if(!targetPath.exists())
        targetPath.mkdir(targetPath.absolutePath() + targetName);

    positiveInfoFileName = QString("%1/%2").arg(targetPath.absolutePath()).arg("pos.txt");

    pictureIt = new QDirIterator(positiveFolderPath,QDir::Files);
    QDirIterator it(positiveFolderPath,QDir::Files);
    while(it.hasNext()){
        it.next();
        NumberOfPositiveImage++;
    }

    // check if there's file exist before, if so, append to it instead of making new file.
    QFile file;
    file.setFileName(positiveInfoFileName);
    NumberOfPositiveImageDisplayed = 0;
    if(!file.exists()){
        statusBar->showMessage("New Object");
    } else if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        QString line;
        while (!in.atEnd()) {
            line = in.readLine();
            processedImageNumber++;
            pictureIt->next();
        }
        NumberOfPositiveImageDisplayed = processedImageNumber;
        statusBar->showMessage("Existed " + QString::number(NumberOfPositiveImageDisplayed) + " objects in pos.txt");

    } else {
        QMessageBox::critical(this,"Error","pos.txt exists, but is not readable");
    }
    file.close();

    if(processedImageNumber < NumberOfPositiveImage){
        displayPositiveImage();
        positiveImageLoaded = true;
        NumberOfSelectedObject =0;
    }else{
        ui->plainTextEdit->appendPlainText("<All postive images have been processed>");
    }
}

//update picture Iterator
void XML_creator::updatePictureIterator(){
    if(pictureIt != NULL){
       delete pictureIt;
       pictureIt = NULL;
    }
    pictureIt = new QDirIterator(QFileInfo(positiveimgpath).absoluteDir());
}


//part 1:display and output positive image information to ***infor.txt file
//part 2:move on to the next positive image
void XML_creator::mouseDoubleClickEvent(QMouseEvent *event){
    if(event->buttons()==Qt::LeftButton
          && mouseIsOnPixmap
          && NumberOfPositiveImageDisplayed < NumberOfPositiveImage
          && NumberOfSelectedObject !=0
          && !stopAllOtherFunction){
        if(pictureIt == NULL){
            event->ignore();
            qDebug() << "picture null";
            return;
        }
        QFile file(positiveInfoFileName);
        if (!file.open(QIODevice::Append | QIODevice::Text)){
            qDebug()<<"could not open file";
        }
        ui->plainTextEdit->appendPlainText(QString("%1 %2 %3").arg(positiveimgpath).arg(NumberOfSelectedObject).arg(PositiveInfo));
        QTextStream out(&file);
        out << QString("%1 %2 %3%4").arg(positiveimgpath).arg(NumberOfSelectedObject).arg(PositiveInfo).arg("\n");
        file.close();

        if(pictureIt->hasNext() ){
            event->accept();
            displayPositiveImage();
        }

        PositiveInfo.clear();
        NumberOfSelectedObject=0;
        NumberOfPositiveImageDisplayed++;
        if(NumberOfPositiveImageDisplayed==(NumberOfPositiveImage)){
            QString positiveResult = QString("%1 %2 %3").arg("Total of").arg( NumberOfPositiveImage).arg(" positive image are proccessed");
            ui->plainTextEdit->appendPlainText(positiveResult);
            ui->plainTextEdit->appendPlainText("-----------Proccess Completed-----------");
        }
    }
}

//update the displayed image size as the XML_creator changes
void XML_creator::resizeEvent(QResizeEvent *event){
    if(positiveimgpath.isEmpty()){
        event->ignore();
    }else{
        QSize size = ui->label_displayposi->size();
        ui->label_displayposi->setPixmap(QPixmap::fromImage(posdisplay).scaled(size,Qt::KeepAspectRatio)); //keep picture size ratio
        displayRect = false;
        event->accept();
        pictureRatio = double(QImage(positiveimgpath).width())/double(ui->label_displayposi->pixmap()->width());
    }
}

//display a positve image on the XML_creator
void XML_creator::displayPositiveImage(){
    if(pictureIt->hasNext()){
        positiveimgpath = pictureIt->next();
        posdisplay = QImage(positiveimgpath);

        posdisplay_px = QPixmap::fromImage(posdisplay);
        QSize size = ui->label_displayposi->size();
        ui->label_displayposi->setPixmap(QPixmap::fromImage(posdisplay).scaled(size,Qt::KeepAspectRatio)); //keep picture size ratio
        pictureRatio = double(QImage(positiveimgpath).width())/double(ui->label_displayposi->pixmap()->width());
    }else{
        ui->label_displayposi->acceptDrops();
    }
}

//mouse press event->getting initial position of the rectangle
void XML_creator::mousePressEvent(QMouseEvent *event){

    if(!stopAllOtherFunction&& event->button()==Qt::LeftButton){
        qpPixmapDimension.setX(0);
        qpPixmapDimension.setY(0);
        qpPixmapInitial = event->pos() - getPixmapTopleftPos();
        qpMainInitial = event->pos();
        statusBar->showMessage("");
    }
    if(!stopAllOtherFunction&&event->button()==Qt::RightButton &&qpPixmapDimension.x()!=0 &&qpPixmapDimension.y()!=0){
        qpPixmapDimension.setX(0);
        qpPixmapDimension.setY(0);
        qpPixmapInitial = event->pos() - getPixmapTopleftPos();
        qpMainInitial = event->pos();

        QString cPositiveInfo = QString("%1 %2 %3 %4 ")
                .arg((int)(qpPixmapInitial.x()*pictureRatio))
                .arg((int)(qpPixmapInitial.y()*pictureRatio))
                .arg((int)(qpPixmapDimension.x()*pictureRatio))
                .arg((int)(qpPixmapDimension.y()*pictureRatio));

        PositiveInfo = PositiveInfo + cPositiveInfo;
        NumberOfSelectedObject++;
       if(NumberOfPositiveImageDisplayed < NumberOfPositiveImage){
           statusBar->showMessage("[x y width height] -> [" +cPositiveInfo+"]",5000);
       }
    }
}

//check if mouse position occurs within the pixmap window
void XML_creator::mouseMoveEvent(QMouseEvent *event){
     if(positiveimgpath.isNull())
         return;

     qpPixmapFinal = event->pos() - getPixmapTopleftPos();
     qpMainFinal = event->pos();
     qpPixmapDimension = qpMainFinal-qpMainInitial;

     QPoint p = ui->label_displayposi->mapFromParent(event->pos());
     QSize Correction = (ui->label_displayposi->size() - ui->label_displayposi->pixmap()->size())/2;
     int displayW = ui->label_displayposi->pixmap()->width();
     int displayH = ui->label_displayposi->pixmap()->height();
     QPoint CorrectedP(p.x() - Correction.width(),p.y() - Correction.height());
     if(CorrectedP.x() < 0 || CorrectedP.y() < 0 ||
             CorrectedP.x() > displayW || CorrectedP.y() > displayH)
     {
        mouseIsOnPixmap = false;
        displayRect = false;
     }else{
        mouseIsOnPixmap = true;
        displayRect = true;
     }
}

//get dimensions of the marked positive ojbect
void XML_creator::mouseReleaseEvent(QMouseEvent *event)
{
    if(!stopAllOtherFunction && event->button()==Qt::LeftButton){
        qpPixmapFinal = event->pos() - getPixmapTopleftPos();
        qpMainFinal = event->pos();
        qpPixmapDimension = qpMainFinal-qpMainInitial;
    }
}

//get top-left position of the pixmap window respect to the main window
QPoint XML_creator::getPixmapTopleftPos()
{
    QPoint p;

    if(!stopAllOtherFunction && positiveImageLoaded){
        int i = (ui->label_displayposi->width()-ui->label_displayposi->pixmap()->width())/2+ui->label_displayposi->pos().x();
        int j = (ui->label_displayposi->height()-ui->label_displayposi->pixmap()->height())/2+ui->label_displayposi->pos().y();
        p.setX(i);
        p.setY(j);
    }
    return p;
}

//to mark the positive image
void XML_creator::paintEvent(QPaintEvent *p)
{
    p->accept();
    this->update();
    if(!stopAllOtherFunction
       && mouseIsOnPixmap
       && displayRect
       && positiveImageLoaded){
           posdisplay_px = QPixmap::fromImage(posdisplay);
           QSize size = ui->label_displayposi->size();
           posdisplay_px = posdisplay_px.scaled(size,Qt::KeepAspectRatio);
           QPainter painter(&posdisplay_px);
           painter.setPen(Qt::blue);
           QRect rec(qpPixmapInitial.x(),qpPixmapInitial.y(),qpPixmapDimension.x(),qpPixmapDimension.y());
           painter.drawRect(rec);
           ui->label_displayposi->setPixmap(posdisplay_px);
     }
}

void XML_creator::on_pushButton_XML_clicked()
{
    stopAllOtherFunction = true;

    QDir workingDir(CASCADETRAININGFOLDER);
    if(!workingDir.exists()){
        QMessageBox::critical(this,"Error","cascade_training folder not exists");
        return;
    }

    resetCascadeFolder();
    createPositiveImageVector();
    haarTraining();
}

void XML_creator::getProcessOutput()
{
    statusBar->showMessage("Generating XML: cascades creation");
    ui->plainTextEdit->appendPlainText(haarTrain->readAll());
}

void XML_creator::generateFileXML(const int &result)
{

    if(result == QProcess::CrashExit)
    {
        statusBar->showMessage("");
        ui->plainTextEdit->appendPlainText("Vector creation is completed!");
        return;
    }  else {
        ui->plainTextEdit->appendPlainText("Cascade files are ready for XML creation!");
    }

    QProcess *generateXML = new QProcess(this);
    QString generateXMLprogram = CASCADETRAININGFOLDER + "/haarconv.exe";
    QStringList xmlArgument;
    xmlArgument << tempCascadePath
                << QString("%1%2").arg(targetPath.absolutePath()).arg(targetName + "xml")
                << "24"
                << "24";
    generateXML->start(generateXMLprogram,xmlArgument);
}

void XML_creator::createPositiveImageVector()
{
    statusBar->showMessage("Generating XML: vector creation");
    QProcess *createVector = new QProcess(this);
    QString vectorProgram = CASCADETRAININGFOLDER + "/createsamples.exe";
    QStringList vectorArgument;
    vecPath = QString("%1/%2").arg(targetPath.absolutePath()).arg("vec.vec");
    vectorArgument<<"-info"<< positiveInfoFileName
                  <<"-vec" << vecPath
                  <<"-num" << "204"
                  <<"-w"   << "24"
                  <<"-h"   << "24";
    createVector->start(vectorProgram,vectorArgument);
    createVector->waitForFinished(-1);

}

void XML_creator::haarTraining()
{
    haarTrain = new QProcess(this);

    connect(haarTrain,SIGNAL(readyRead()),this,SLOT(getProcessOutput()));
    connect(haarTrain,SIGNAL(finished(int)),this,SLOT(generateFileXML(int)));
    QString haarTrainingProgram = CASCADETRAININGFOLDER + "/haartraining.exe";
    QStringList haarTrainArgument;

    QDir cascade(tempCascadePath);
    if(!cascade.exists())
        cascade.mkdir(tempCascadePath);

    haarTrainArgument<< "-data"<< tempCascadePath
                     << "-vec"<< vecPath
                     << "-bg" << negativeInfoFileName
                     << "-npos"<< QString::number(NumberOfPositiveImageDisplayed)
                     << "-nneg"<< QString::number(numberofn)
                     << "-nstages"<< "15"
                     << "-mem"<<"1024"
                     << "-mode"<< "ALL"
                     << "-w"<< "24"
                     << "-h"<< "24"
                     << "rem"<<"-nonsym";
    haarTrain->start(haarTrainingProgram,haarTrainArgument);
    haarTrain->setReadChannelMode(QProcess::ForwardedChannels);
}

void XML_creator::resetCascadeFolder()
{
    tempCascadePath = QString("%1/%2").arg(targetPath.absolutePath()).arg("cascades");
    QDir cascade(tempCascadePath);
    cascade.removeRecursively();
}

