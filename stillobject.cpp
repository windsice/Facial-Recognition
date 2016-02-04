#include "stillobject.h"
#include "ui_stillobject.h"

StillObject::StillObject(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StillObject)
{
    ui->setupUi(this);
}

StillObject::~StillObject()
{
    delete ui;
}

void StillObject::displayOnGUI(){
    setImage(PicturePath);
    performDetection();
    QSize size = ui->label_StillObject->size();
    ui->label_StillObject->setPixmap(QPixmap::fromImage(*imgForDisplay).scaled(size,Qt::KeepAspectRatio));
}

void StillObject::updateFolderIterator(){

    if(folderIt != NULL){
        delete folderIt;
        folderIt = NULL;
    }
    folderIt = new QDirIterator(QFileInfo(PicturePath).absoluteDir());
}

void StillObject::resizeEvent(QResizeEvent *event){

    // if the path is empty then there's no imgfordisplay.
    if(PicturePath.isEmpty()){
        event->ignore();
    } else {
        QSize size = ui->label_StillObject->size();
        ui->label_StillObject->setPixmap(QPixmap::fromImage(*imgForDisplay).scaled(size,Qt::KeepAspectRatio));
        event->accept();
    }
}

void StillObject::mousePressEvent(QMouseEvent *event){
    if(folderIt == NULL){
        event->ignore();
        return;
    }

    if(folderIt->hasNext()){
        event->accept();
        PicturePath = folderIt->next();
        qDebug() << PicturePath;
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
