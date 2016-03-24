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
}

XML_creator::~XML_creator()
{
    delete ui;
}

//display negative image list and write the list to a ****bg.txt file
void XML_creator::get_negative_image_list(){
    QDirIterator it(negativeFolderPath,QDir::Files);
    int numberofn = 0;
    negativeInfoFileName = QString("%1%2").arg(targetPath.absolutePath()).arg("neg.txt");

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
    processedImageNumber = 0;
    int placeToStopScanPositiveFile = 0;
    NumberOfPositiveImage = 0;
    QFile file;

    if(!targetPath.exists())
        targetPath.mkdir(targetPath.absolutePath() + targetName);

    positiveInfoFileName = QString("%1%2").arg(targetPath.absolutePath()).arg("pos.txt");
    file.setFileName(positiveInfoFileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        //qDebug()<<"could not open file-------";

    }
        QTextStream in(&file);
        QString line;
        while (!in.atEnd()) {
            line = in.readLine();
            processedImageNumber++;
        }

        file.close();
        QDirIterator it(positiveFolderPath,QDir::Files);
        while(it.hasNext()){
            it.next();
            NumberOfPositiveImage ++;
        }
        NumberOfPositiveImageDisplayed = processedImageNumber;
        pictureIt = new QDirIterator(positiveFolderPath,QDir::Files);
        while(placeToStopScanPositiveFile < processedImageNumber){
            pictureIt->next();
            placeToStopScanPositiveFile++;
        }
    if(processedImageNumber<NumberOfPositiveImage){
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
      && !stopAllOtherFunction
            ){
            if(pictureIt == NULL){
                event->ignore();
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
    }
    if(!stopAllOtherFunction&&event->button()==Qt::RightButton &&qpPixmapDimension.x()!=0 &&qpPixmapDimension.y()!=0){

            QString cPositiveInfo = QString("%1 %2 %3 %4 ")
                    .arg((int)(qpPixmapInitial.x()*pictureRatio))
                    .arg((int)(qpPixmapInitial.y()*pictureRatio))
                    .arg((int)(qpPixmapDimension.x()*pictureRatio))
                    .arg((int)(qpPixmapDimension.y()*pictureRatio));

            PositiveInfo = PositiveInfo + cPositiveInfo;
            NumberOfSelectedObject++;
           if(NumberOfPositiveImageDisplayed < NumberOfPositiveImage){
            //ui->statusBar->showMessage("[x y width height] -> [" +cPositiveInfo+"]",5000);
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

     if( qpMainInitial.ry()>=getPixmapTopleftPos().ry()
     && qpMainInitial.rx()>=272+(ui->label_displayposi->width() - ui->label_displayposi->pixmap()->width())/2
     && qpMainFinal.ry()>=getPixmapTopleftPos().ry()
     && qpMainFinal.rx()>=272+(ui->label_displayposi->width() - ui->label_displayposi->pixmap()->width())/2
     && qpMainInitial.ry()<=(getPixmapTopleftPos().ry()+ui->label_displayposi->pixmap()->height())
     && qpMainInitial.rx()<=(272+ui->label_displayposi->pixmap()->width()+(ui->label_displayposi->width() - ui->label_displayposi->pixmap()->width())/2)
     && qpMainFinal.ry()<=(getPixmapTopleftPos().ry()+ui->label_displayposi->pixmap()->height())
     && qpMainFinal.rx()<=(272+ui->label_displayposi->pixmap()->width())+(ui->label_displayposi->width() - ui->label_displayposi->pixmap()->width())/2
        ){
            mouseIsOnPixmap = true;
            displayRect = true;
         }else{
            mouseIsOnPixmap = false;
            displayRect = false;
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
        int i = ui->label_displayposi->pos().x()+(ui->label_displayposi->width() - ui->label_displayposi->pixmap()->width())/2;
        int j = (ui->label_displayposi->height() - ui->label_displayposi->pixmap()->height())/2+25;
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
    resetCascadeFolder();
    createPositiveImageVector();
    haarTraining();
}

void XML_creator::getProcessOutput()
{
    //ui->statusBar->showMessage("Generating XML: cascades creation");
    ui->plainTextEdit->appendPlainText(haarTrain->readAll());
}

void XML_creator::generateFileXML(const int &result)
{
    if(result == QProcess::CrashExit)
    {
        //ui->statusBar->showMessage("");
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
    //ui->statusBar->showMessage("Generating XML: vector creation");
    QProcess *createVector = new QProcess(this);
    QString vectorProgram = CASCADETRAININGFOLDER + "/createsamples.exe";
    QStringList vectorArgument;
    vecPath = QString("%1%2").arg(targetPath.absolutePath()).arg("vec.vec");
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
    tempCascadePath = QString("%1%2").arg(targetPath.absolutePath()).arg("cascades");
    QDir cascade(tempCascadePath);
    if(!cascade.exists())
        cascade.mkdir(tempCascadePath);

    haarTrainArgument<< "-data"<< tempCascadePath
                     << "-vec"<< vecPath
                     << "-bg" << negativeInfoFileName
                     << "-npos"<< "200"
                     << "-nneg"<< "200"
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
    QDir cascade(tempCascadePath);
    cascade.removeRecursively();
}

