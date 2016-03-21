#include "recognizer.h"
#include "ui_recognizer.h"
#include "QDebug"

const QList<QSize> Recognizer::RESOLUTION = QList<QSize> ()
    << QSize(600,480)
    << QSize(800,600)
    << QSize(1024,768)
    << QSize(1280,1024)
    << QSize(1680,1050)
    << QSize(1920,1080)
    << QSize(2160,1440);

Recognizer::Recognizer(QWidget *parent) :
    QMainWindow(parent),
    settingsUI(new Ui::Recognizer)
{
    settingsUI->setupUi(this);
    LoadSettings();
    Initialization();
    onOpSel();
}

Recognizer::~Recognizer()
{
    delete statProgress;
    delete detector;
    delete settingsUI;
}

//turn visibility based on user's intended operation
void Recognizer::onOpSel(){

    settingsUI->groupBox_CamSet->setVisible(false);
    settingsUI->groupBox_preparation->setVisible(false);
    settingsUI->groupBox_ObjDet->setVisible(false);
    settingsUI->groupBox_FacialRec->setVisible(false);
    settingsUI->groupBox_prediction->setVisible(false);
    settingsUI->groupBox_colorHasPath->setVisible(false);

    if(settingsUI->pushButton_liveFacial->isChecked()){

        settingsUI->groupBox_CamSet->setVisible(true);
        settingsUI->groupBox_preparation->setVisible(true);
        settingsUI->groupBox_ObjDet->setVisible(true);
        settingsUI->groupBox_FacialRec->setVisible(true);
        StackWidgetIndex = TAB_CAMERA;

    } else if(settingsUI->pushButton_stillFacial->isChecked()){

        settingsUI->groupBox_preparation->setVisible(true);
        settingsUI->groupBox_ObjDet->setVisible(true);
        settingsUI->groupBox_FacialRec->setVisible(true);
        settingsUI->groupBox_prediction->setVisible(true);
        StackWidgetIndex = TAB_SETTING;
        
    } else if(settingsUI->pushButton_stillObject->isChecked()){

        settingsUI->groupBox_preparation->setVisible(true);
        settingsUI->groupBox_ObjDet->setVisible(true);
        StackWidgetIndex = TAB_STILLOBJECT;
    } else if(settingsUI->pushButton_XMLcreator->isChecked()){
        StackWidgetIndex = TAB_XMLCREATOR;
    } else {
        StackWidgetIndex = TAB_SETTING;
    }
}

bool Recognizer::passParaToOp(){
    if(settingsUI->stackedWidget->currentIndex() == StackWidgetIndex)
        return false;

    settingsUI->stackedWidget->setCurrentIndex(StackWidgetIndex);
    if(StackWidgetIndex == TAB_CAMERA)
    {
        detector->setCameraDevice(settingsUI->CameraNumber->value());
        detector->setCameraResolution(settingsUI->comboBox_CameraResolution->currentData().toSize());
        detector->setModels(settingsUI->LBPH_checkBox->isChecked(),LBPHModel,settingsUI->EigenFace_checkBox->isChecked(),EigenFaceModel,settingsUI->FisherFace_checkBox->isChecked(),FisherFaceModel);
        detector->setImageHeight(img_height);
        detector->setImageWidth(img_width);
        detector->setUpdatingTime(settingsUI->CameraUpdateTime->value());
        detector->setClassifierDuration(settingsUI->ClassifierDuration->value());
        if(!ClassifierPath.isEmpty()){
            try{
            detector->setClassifier(ClassifierPath);
            } catch( Exception e)
            {
                QMessageBox::critical(this,"Error","Classifier, Something wrong with the classifier");
            }
        }
        detector->Capturing();
    }
    else if(StackWidgetIndex == TAB_STILLOBJECT){
        if(!ClassifierPath.isEmpty())
            stillObject->setClassifier(ClassifierPath);
        if(!ColorClassifierPath.isEmpty())
            stillObject->setColorClassifier(ColorClassifierPath);
    }
    else
    {
        detector->Stop();
    }
    return true;
}

void Recognizer::Initialization()
{
    //create a progressbar next to the status bar
    statProgress = new QProgressBar(this);
    statProgress->setMaximumWidth(120);
    settingsUI->statusBar->addPermanentWidget(statProgress);\
    statProgress->setTextVisible(false);

    //Initialize FaceRecognizer models
    LBPHModel = createLBPHFaceRecognizer();
    EigenFaceModel = createEigenFaceRecognizer();
    FisherFaceModel = createFisherFaceRecognizer();

    //Update GUI
    on_LBPH_checkBox_clicked(false);
    on_EigenFace_checkBox_clicked(false);
    on_FisherFace_checkBox_clicked(false);
    on_UpdateExisting_checkBox_clicked(false);

    //For TrainingThread communication
    connect(this,SIGNAL(TrainingObjects(QString)),this,SLOT(onTrainingObjects(QString)));

    TabsInit();

    // Camera Resolution
    foreach(QSize a, RESOLUTION)
    {
        QString text = QString::number(a.width()) + " x " + QString::number(a.height());
        settingsUI->comboBox_CameraResolution->addItem(text,QVariant(a));
    }

    img_height = img_width = 0;
    StackWidgetIndex = TAB_SETTING;
}

//construct all other tabs
void Recognizer::TabsInit(){
    //Camera
    detector = new Detector(this);
    settingsUI->stackedWidget->insertWidget(TAB_CAMERA,detector);
    settingsUI->pushButton_setting->setVisible(false);
    QPalette Pal;
    Pal.setColor(QPalette::Background,Qt::black);
    settingsUI->stackedWidget->widget(TAB_CAMERA)->setAutoFillBackground(true);
    settingsUI->stackedWidget->setPalette(Pal);

    //stillObject
    stillObject = new StillObject(this);
    settingsUI->stackedWidget->insertWidget(TAB_STILLOBJECT,stillObject);

    //xml creator
    xml_creator = new XML_creator(this);
    settingsUI->stackedWidget->insertWidget(TAB_XMLCREATOR,xml_creator);
}

Mat Recognizer::norm_0_255(InputArray _src) {
    Mat src = _src.getMat();
    // Create and return normalized image:
    Mat dst;
    switch(src.channels()) {
    case 1:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
        break;
    case 3:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
        break;
    default:
        src.copyTo(dst);
        break;
    }
    return dst;
}

//given the locations of files, put those in vector<Mat> and vector<int> for later training
void Recognizer::read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator) {
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
    img_width = images[0].cols;
    img_height = images[0].rows;
}

//For Other thread to train the model, prevents freezing GUI
void Recognizer::Training()
{
    //For communicating the main thread. (update GUI)
    emit Recognizer::TrainingObjects("Begin");

    if(settingsUI->LBPH_checkBox->isChecked())
    {
        emit Recognizer::TrainingObjects("LBPH");
        LBPHModel->train(images,labels);
    }

    if(settingsUI->EigenFace_checkBox->isChecked())
    {
        emit Recognizer::TrainingObjects("EigenFace");
        EigenFaceModel->train(images, labels);
    }

    if(settingsUI->FisherFace_checkBox->isChecked())
    {
        emit Recognizer::TrainingObjects("FisherFace");
        FisherFaceModel->train(images, labels);
    }

    emit Recognizer::TrainingObjects("End");

}

//save path (mostly) for next time
void Recognizer::SaveSettings()
{
    QSettings settings("Settings.ini",QSettings::IniFormat);
    settings.beginGroup("FileLocation");
    settings.setValue("Target Path",Target_Path);
    settings.setValue("CVS Path",CVS_Path);
    settings.setValue("Classifier Path",ClassifierPath);
    if(settingsUI->LBPH_checkBox->isChecked())
    settings.setValue("LBPH Path",LBPHPath);
    if(settingsUI->EigenFace_checkBox->isChecked())
    settings.setValue("EigenFace Path",EigenFacePath);
    if(settingsUI->EigenFace_checkBox->isChecked())
    settings.setValue("FisherFace Path",FisherFacePath);
    settings.endGroup();

}

//load path from last time
void Recognizer::LoadSettings()
{
    QSettings settings("Settings.ini",QSettings::IniFormat);
    settings.beginGroup("FileLocation");
    settingsUI->Target_lineEdit->setText(settings.value("Target Path").toString());
    settingsUI->CVS_Path_lineEdit->setText(settings.value("CVS Path").toString());
    settingsUI->ClassifierLineEdit->setText(settings.value("Classifier Path").toString());
    settingsUI->LBPH_lineEdit->setText(settings.value("LBPH Path").toString());
    settingsUI->EigenFace_location_lineEdit->setText(settings.value("EigenFace Path").toString());
    settingsUI->FisherFace_location_lineEdit->setText(settings.value("FisherFace Path").toString());
    settings.endGroup();
}

//get CVS Path
void Recognizer::on_CVS_Path_Button_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getOpenFileName(this,"Select CVS File",CVS_Path,"Text Files (*.txt)");
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        CVS_Path = tempPath;
        settingsUI->CVS_Path_lineEdit->setText(CVS_Path);
    }
}

void Recognizer::on_CVS_Path_lineEdit_textChanged(const QString &arg1)
{
    CVS_Path = arg1;
}

//predict the person in given image from existing labels.
void Recognizer::on_Predict_Button_clicked()
{
    SaveSettings();
    try{
    testSample = imread(Target_Path.toStdString(),0);
    } catch (Exception e)
    {
        QMessageBox::critical(this,"ERROR","Check your path");
    }

    String result_message;
    try{
        if(settingsUI->LBPH_checkBox->isChecked())
        {
            int LBPHPredictedLabel = LBPHModel->predict(testSample);
            result_message.append("LBPH Predicted Class = ");
            result_message.append(QString::number(LBPHPredictedLabel).toStdString());
            result_message.append("\t");
        }

        if(settingsUI->EigenFace_checkBox->isChecked())
        {
            int EigenFacePredictedLabel = EigenFaceModel->predict(testSample);
            result_message.append("EigenFace Predicted Class = ");
            result_message.append(QString::number(EigenFacePredictedLabel).toStdString());
            result_message.append("\t");
        }

        if(settingsUI->FisherFace_checkBox->isChecked())
        {
            int FisherFacePredictedLabel = FisherFaceModel->predict(testSample);
            result_message.append("FisherFace Predicted Class = ");
            result_message.append(QString::number(FisherFacePredictedLabel).toStdString());
            result_message.append("\t");
        }
    } catch (Exception e)
    {
        QMessageBox::critical(this,"ERROR","Did you train it yet?");
    }

    settingsUI->Predition_Result->setText(QString::fromStdString(result_message));
}

//start training by running another thread.
void Recognizer::on_Train_Button_clicked()
{
    SaveSettings();

    // Get the path to your CSV.
    string fn_csv = CVS_Path.toStdString();
    // These vectors hold the images and corresponding labels.
    // Read in the data. This can fail if no valid
    // input filename is given.
    try {
       read_csv(fn_csv, images, labels);
    } catch (cv::Exception& e) {
       cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
       // nothing more we can do
       exit(1);
    }
    // Quit if there are not enough images for this demo.
    if(images.size() <= 1) {
       string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
       cout << error_message;
    }

    QFuture<void> t1 = QtConcurrent::run(this,&Recognizer::Training);
}

//For training thread use, to show the progress of training.
void Recognizer::onTrainingObjects(QString type)
{
    if(type == "Begin")
    {
        statProgress->setValue(100);
    }
    else if(type == "LBPH")
        settingsUI->statusBar->showMessage("Training LBPH..");
    else if(type == "EigenFace")
        settingsUI->statusBar->showMessage("Training EigenFace..");
    else if(type == "FisherFace")
        settingsUI->statusBar->showMessage("Training FisherFace..");
    else
    {
        settingsUI->statusBar->showMessage("");
        statProgress->setValue(0);
    }
}

//TO create a cvs file
void Recognizer::on_CreateCVS_Button_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getExistingDirectory(this,"Picture Database Directory",CVS_Path,QFileDialog::ShowDirsOnly);
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        QString cvspath;
        cvspath = QFileDialog::getSaveFileName(this,"Ouput Location for CVS",tempPath,"Text Files(*.txt)");

        if(QFileDialog::Accepted && !cvspath.isNull())
        {
            QFile file(cvspath);
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            QTextStream output(&file);

            QDirIterator Folders(tempPath,QDir::AllDirs | QDir::NoDot | QDir::NoDotDot);
            int label = 1;
            while(Folders.hasNext())
            {
                QDirIterator Files(Folders.next(),QDir::Files | QDir::NoDot | QDir::NoDotDot);
                while(Files.hasNext())
                {
                    output << Files.next() << ";" << label << "\n";
                }
                label++;
            }

            file.close();
            settingsUI->statusBar->showMessage(QFileInfo(cvspath).fileName() + " Created Successfully",3000);
            settingsUI->CVS_Path_lineEdit->setText(file.fileName());
        }
    }
}

void Recognizer::on_Target_lineEdit_textChanged(const QString &arg1)
{
    Target_Path = arg1;
}

void Recognizer::on_Target_toolButton_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getOpenFileName(this,"Select Target Image",Target_Path,"Images (*.pgm)");
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        Target_Path = tempPath;
        settingsUI->Target_lineEdit->setText(Target_Path);
    }
}

void Recognizer::on_LBPH_lineEdit_textChanged(const QString &arg1)
{
    LBPHPath = arg1;
}

void Recognizer::on_EigenFace_location_lineEdit_textChanged(const QString &arg1)
{
    EigenFacePath = arg1;
}

void Recognizer::on_FisherFace_location_lineEdit_textChanged(const QString &arg1)
{
    FisherFacePath = arg1;
}

//for GUI, if the algorithm is not use, its components are not available for use.
void Recognizer::on_LBPH_checkBox_clicked(bool checked)
{
    if(checked == true)
    {
        settingsUI->LBPH_lineEdit->setEnabled(true);
        settingsUI->LBPH_toolButton->setEnabled(true);
        settingsUI->groupBox_LBPHupdate->setVisible(true);
    }else
    {
        settingsUI->LBPH_lineEdit->setEnabled(false);
        settingsUI->LBPH_toolButton->setEnabled(false);
        settingsUI->groupBox_LBPHupdate->setVisible(false);
    }
    AlgorithmChecked();
}

//for GUI, if the algorithm is not use, its components are not available for use.
void Recognizer::on_EigenFace_checkBox_clicked(bool checked)
{
    if(checked == true)
    {
        settingsUI->EigenFace_location_lineEdit->setEnabled(true);
        settingsUI->EigenFace_location_toolButton->setEnabled(true);
    }else
    {
        settingsUI->EigenFace_location_lineEdit->setEnabled(false);
        settingsUI->EigenFace_location_toolButton->setEnabled(false);
    }
    AlgorithmChecked();
}

//for GUI, if the algorithm is not use, its components are not available for use.
void Recognizer::on_FisherFace_checkBox_clicked(bool checked)
{
    if(checked == true)
    {
        settingsUI->FisherFace_location_lineEdit->setEnabled(true);
        settingsUI->FisherFace_location_toolButton->setEnabled(true);
    }else
    {
        settingsUI->FisherFace_location_lineEdit->setEnabled(false);
        settingsUI->FisherFace_location_toolButton->setEnabled(false);
    }
    AlgorithmChecked();
}

//for GUI, if the algorithms are not use, their components are not available for use.
void Recognizer::AlgorithmChecked()
{
    if(settingsUI->LBPH_checkBox->isChecked() || settingsUI->EigenFace_checkBox->isChecked() || settingsUI->FisherFace_checkBox->isChecked())
    {
        settingsUI->Train_Button->setEnabled(true);
        settingsUI->Predict_Button->setEnabled(true);
        settingsUI->SaveTrainedModel_pushButton->setEnabled(true);
        settingsUI->LoadTrainedModel_pushButton->setEnabled(true);
    }else{
        settingsUI->Train_Button->setEnabled(false);
        settingsUI->Predict_Button->setEnabled(false);
        settingsUI->SaveTrainedModel_pushButton->setEnabled(false);
        settingsUI->LoadTrainedModel_pushButton->setEnabled(false);
    }
}

//For loading/saving model
void Recognizer::on_LBPH_toolButton_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getSaveFileName(this,"Save or Select LBPH File",LBPHPath,"XML (*.xml);;YAML (*.YAML)");
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        LBPHPath = tempPath;
        settingsUI->LBPH_lineEdit->setText(tempPath);
    }
}

//For loading/saving model
void Recognizer::on_EigenFace_location_toolButton_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getSaveFileName(this,"Save or Select EigenFace File",EigenFacePath,"XML (*.xml);;YAML (*.YAML)");
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        EigenFacePath = tempPath;
        settingsUI->EigenFace_location_lineEdit->setText(tempPath);
    }
}

//For loading/saving model
void Recognizer::on_FisherFace_location_toolButton_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getSaveFileName(this,"Save or Select FisherFace File",FisherFacePath,"XML (*.xml);;YAML (*.YAML)");
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        FisherFacePath = tempPath;
        settingsUI->FisherFace_location_lineEdit->setText(tempPath);
    }
}

//For loading/saving model
void Recognizer::on_SaveTrainedModel_pushButton_clicked()
{

    settingsUI->statusBar->showMessage("Saving models..");
    statProgress->setValue(100);

    if(settingsUI->LBPH_checkBox->isChecked())
        LBPHModel->save(LBPHPath.toStdString());

    if(settingsUI->EigenFace_checkBox->isChecked())
        EigenFaceModel->save(EigenFacePath.toStdString());

    if(settingsUI->FisherFace_checkBox->isChecked())
        FisherFaceModel->save(FisherFacePath.toStdString());

    settingsUI->statusBar->showMessage("");
    statProgress->setValue(0);
}

//For loading/saving model
void Recognizer::on_LoadTrainedModel_pushButton_clicked()
{
    settingsUI->statusBar->showMessage("Loading models..");
    statProgress->setValue(100);

    try{
        if(settingsUI->LBPH_checkBox->isChecked())
            LBPHModel->load(LBPHPath.toStdString());
        if(settingsUI->EigenFace_checkBox->isChecked())
            EigenFaceModel->load(EigenFacePath.toStdString());

        if(settingsUI->FisherFace_checkBox->isChecked())
            FisherFaceModel->load(FisherFacePath.toStdString());
    } catch (Exception e)
    {
        QMessageBox::critical(this,"ERROR","Invalid Path");
    }

    settingsUI->statusBar->showMessage("");
    statProgress->setValue(0);
}

//for Updating LBPH
void Recognizer::on_UpdateExisting_checkBox_clicked(bool checked)
{
    if(checked == true)
    {
        settingsUI->UpdateExisting_spinBox->setEnabled(true);
        settingsUI->UpdateLocation_lineEdit->setEnabled(true);
        settingsUI->UpdateLocation_toolButton->setEnabled(true);
        settingsUI->TrainNew_pushButton->setEnabled(true);
    }else{
        settingsUI->UpdateExisting_spinBox->setEnabled(false);
        settingsUI->UpdateLocation_lineEdit->setEnabled(false);
        settingsUI->UpdateLocation_toolButton->setEnabled(false);
        settingsUI->TrainNew_pushButton->setEnabled(false);
    }
}

//for Updating LBPH
void Recognizer::on_UpdateLocation_lineEdit_textChanged(const QString &arg1)
{
   UpdateExistingDirectory = arg1;
}

//for Updating LBPH
void Recognizer::on_UpdateLocation_toolButton_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getExistingDirectory(this,"Updating Directory",UpdateExistingDirectory,QFileDialog::ShowDirsOnly);
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        UpdateExistingDirectory = tempPath;
        settingsUI->UpdateLocation_lineEdit->setText(UpdateExistingDirectory);
    }
}

//for Updating LBPH
void Recognizer::on_TrainNew_pushButton_clicked()
{
    QString cvspath = QDir::currentPath() + "/updating.txt";

    QFile file(cvspath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream output(&file);

    QDirIterator Files(UpdateExistingDirectory,QDir::Files | QDir::NoDot | QDir::NoDotDot);
    int label = settingsUI->UpdateExisting_spinBox->value();
    while(Files.hasNext())
    {
        output << Files.next() << ";" << label << "\n";
    }

    file.close();

    try{
        string fn_csv = cvspath.toStdString();
        if(fn_csv != "")
        {
            vector<Mat> updatingImages;
            vector<int> updatingLabels;

            try {
               read_csv(fn_csv, updatingImages, updatingLabels);
            } catch (cv::Exception& e) {
               cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
               // nothing more we can do
               exit(1);
            }

            LBPHModel->update(updatingImages,updatingLabels);
        }
    } catch(Exception e)
    {
        QMessageBox::critical(this,"ERROR","Check your location");
    }
}

void Recognizer::on_ClassifierLineEdit_textChanged(const QString &arg1)
{
    ClassifierPath = arg1;
}

void Recognizer::on_ClassifierPath_toolButton_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getOpenFileName(this,"Select Classifier XML Files",ClassifierPath,"XML Files (*.xml)");
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        ClassifierPath = tempPath;
        settingsUI->ClassifierLineEdit->setText(ClassifierPath);
    }
}

void Recognizer::on_lineEdit_colorClassifier_textChanged(const QString &arg1)
{
    if(arg1.isNull() || arg1.isEmpty())
        return;

    ColorClassifierPath = arg1;
    settingsUI->groupBox_colorHasPath->setVisible(true);
}

void Recognizer::on_toolButton_colorClassifier_clicked()
{
    QString tempPath;
    tempPath = QFileDialog::getOpenFileName(this,"Select Color Classifier XML Files",ColorClassifierPath,"XML Files (*.xml)");
    if(QFileDialog::Accepted && !tempPath.isNull())
    {
        ColorClassifierPath = tempPath;
        settingsUI->lineEdit_colorClassifier->setText(ColorClassifierPath);

    }
}

void Recognizer::on_pushButton_liveFacial_clicked()
{
    onOpSel();
}

void Recognizer::on_pushButton_stillFacial_clicked()
{
    onOpSel();
}

void Recognizer::on_pushButton_stillObject_clicked()
{
    onOpSel();
}

//the blue right arrow on the bottom
void Recognizer::on_pushButton_action_clicked()
{
    if(passParaToOp()){
        settingsUI->pushButton_action->setVisible(false);
        settingsUI->pushButton_setting->setVisible(true);
    }
}

//the blue left arrow on the bottom, which always back to setting when clicked
void Recognizer::on_pushButton_setting_clicked()
{
    StackWidgetIndex = TAB_SETTING;
    if(passParaToOp()){
        settingsUI->pushButton_action->setVisible(true);
        settingsUI->pushButton_setting->setVisible(false);
    }
    onOpSel();
}

void Recognizer::on_pushButton_XMLcreator_clicked()
{
    onOpSel();
}
