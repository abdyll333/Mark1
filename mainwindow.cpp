#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <omp.h>
#include <QTime>
#include <QMessageBox>

void Controller::handleResults()
{
  qDebug()<<worker->getNeuronSign()<< "finished with runs:"<< worker->getRunCount() <<" time( "<< QTime().currentTime() <<" )";
  QString printInfo("{ Good: %1 ;   No itself symb(err): %2 ;   Itself symb(err): %3   }");
  qDebug()<<printInfo.arg(worker->getLastStudyGoodCnt(),5).
                  arg(worker->getLastStudyNDSCnt(),5).
                  arg(worker->getLastStudyDollarCnt(),5)<<endl;
}

void Controller::stoppedHandle()
{
  qDebug()<<worker->getNeuronSign()<< "stopped with runs"<< worker->getRunCount() <<" time( "<< QTime().currentTime() <<" )";
  QString printInfo("{ Good: %1 ;   No itself symb(err): %2 ;   Itself symb(err): %3   }");
  qDebug()<<printInfo.arg(worker->getLastStudyGoodCnt(),5).
                  arg(worker->getLastStudyNDSCnt(),5).
                  arg(worker->getLastStudyDollarCnt(),5)<<endl;
}

void  Controller::sltResultsRecogn(bool res , long sum)
{
      if(res)
           qDebug()<<"Это значение похоже на:"<<static_cast<char>(worker->getNeuronSign())<<" "<< QString("%1").arg(sum,8) <<endl;
    /*  else
           qDebug()<<"Это значение HE похоже на:"<<static_cast<char>(worker->getNeuronSign())<<" "<< QString("%1").arg(sum,8) <<endl;*/
}

void Controller::stopE()
{
  worker->stopCalc();
  workerThread.wait(10);
  workerThread.start();
}

void ZipPicture::sltMakeData(char signUp, char signDown) //todo чекать количество файлов на переполнение буфера
{
  if(_enState ==enIdling)
  {
      _enState = enWorked;
      qDebug() <<"Zip got it" << endl;
      QDir dataDir(_strWorkDirectoryPath);
      QStringList filters;
      filters << "*.png" << "*.jpg" << "*.bmp";
      if(_pDataList && !_strWorkDirectoryPath.isEmpty())
      {
       _pDataList->clear();
       int cnt(0), size(0);

       // #pragma omp parallel for
        for(char sign = signDown; sign <= signUp; sign++)       //full list of files
        {
            QDir dir(dataDir);
            dir.setNameFilters(filters);
            if(dir.cd(_strWorkDirectoryPath+'/'+sign)){
              size += dir.entryList(QDir::Files).size()  ;
            }
        }

       float progr(100.f / size);
    
        long iFullSize(0);

      //  #pragma omp parallel for
        for(char sign = signDown; sign <= signUp; sign++)       //full list of files
        {
            QDir dir(dataDir);
            QImage oQImage;
            QString path("");
            dir.setNameFilters(filters);
            if(dir.cd(_strWorkDirectoryPath+'/'+sign)){

              QStringList lst(dir.entryList(QDir::Files));
              foreach (QString fileinf, lst) {
                  path = dir.absoluteFilePath(fileinf);
                  if(oQImage.load(path)){
                  //  studyData item(sign, path, image);
                    _pDataList->append( studyData(sign, path, oQImage) );
                    iFullSize += oQImage.sizeInBytes();
                    ++cnt;
                  }
                  emit  updateProgress(static_cast<int>(cnt * progr+1));
              }
            }
        }
        qDebug() <<"files cnt" <<size << "size of images:"<< iFullSize << _pDataList->size() <<" "<< cnt <<endl;
        emit resReady(true);
      }
      else
        emit resReady(false);
      _enState = enIdling;
  }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _pFullDataList(new QList<studyData>),
    _iCalcReady(0),
    _iGoodTry(0),
    _iNDSTry(0),
    _iDollarTry(0),
    _upLimitNeiron( chSignFinish ),
    _downLimitNeiron( chSignStart ),
    saveData(new SaveDataFiles(this))
{
    ui->setupUi(this);
    setInitNetValue();

  zipper = new ZipPicture();
  zipper->moveToThread(&zipThread);
  connect(&zipThread, &QThread::finished, zipper, &QObject::deleteLater);
  connect(this, &MainWindow::sigZipFile, zipper, &ZipPicture::sltMakeData);
  connect(zipper, &ZipPicture::resReady, this, &MainWindow::sltTeaching);
  connect(zipper, &ZipPicture::updateProgress, this, &MainWindow::sltUpdateZip);

  //connect(this, &MainWindow::)

  zipThread.start();
  zipThread.setPriority(QThread::LowPriority);

    qDebug()<<omp_get_max_threads()<<endl;
    int nProcs=omp_get_max_threads();
    omp_set_num_threads(nProcs);
    qDebug()<<omp_get_num_threads()<<endl;

    for(int i = 0 ; i <= ciCountRepeats ; ++i)  ////todo переделать под многоязычность
    {
      ui->cbNeiron->addItem(QString(QChar(chStart.unicode()+i)));
      ui->cbSign->addItem(QString(QChar(chStart.unicode()+i)));
    }

    ui->lePath->setText("./Img");
    ui->cbNeiron->setCurrentIndex(0);
    ui->cbSign->setCurrentIndex(0);
    ui->pbPicts->setValue(0);
    ui->pbPicts->setMaximum(100);
    ui->pbStudy_Rec->setValue(0); 
  //  ui->pbPicts->setFormat("%v");
  //  ui->pbStudy_Rec->setFormat("%v");

    ui->spbRounds->setMinimum(1);
    ui->spbRounds->setMaximum(100000);
    ui->spbRounds->setValue(1);

    ui->chbRememberPath->setChecked(true);

 /*   _Timer.setSingleShot(false);
    _Timer.setInterval(1000);                 //todo qthread helloooyyy

    connect(&_Timer, &QTimer::timeout, this, [=] () {
      ui->pbGood->setValue(_iGoodTry % ui->pbGood->maximum());
      ui->pbNDS->setValue(_iNDSTry % ui->pbNDS->maximum());
    });
  _Timer.start();*/

  connect(saveData,SIGNAL(sigSavePict(QChar,QString&)),this,SLOT(sltSavePicts(QChar,QString&)));
  connect(saveData,SIGNAL(sigSaveWeight(QChar,QString&)),this,SLOT(sltSaveWeights(QChar,QString&)));
}

MainWindow::~MainWindow()
{
  foreach(Controller* t,brain)
    delete t;
 /* foreach(neuron* n,fromPinkyToBrain)
    delete n;*/
  brain.clear();
  fromPinkyToBrain.clear();
  zipThread.quit();
  zipThread.wait();
  _pFullDataList->clear();
  delete _pFullDataList;
  delete ui;
}

void MainWindow::on_pushButton_3_clicked()
{
    QApplication::quit();
}

void MainWindow::setInitNetValue()
{
    int count=0;
    for(char sign = chSignStart; sign <= chSignFinish;sign++)
    {
        fromPinkyToBrain.push_back(new neuron(sign));
        brain.push_back(new Controller(fromPinkyToBrain[count],sign));

        connect(this, &MainWindow::sigStopCalc, brain[ count ], &Controller::stopE);
        connect(fromPinkyToBrain[count], &neuron::resultReady, this, &MainWindow::sltAccumRes);
        connect(fromPinkyToBrain[count], &neuron::resultStopped, this, &MainWindow::sltAccumRes);
        count++;
    }

}

void MainWindow::vViewImage()
{
  _strPathTemp=QFileDialog::getOpenFileName(this, tr("Open File"),"/Img/Etalone",tr("Image files(*.*)"));
  if(!_strPathTemp.isEmpty())
  {
    QImage image(_strPathTemp);
    image = image.scaled(32,32);
    _lblImage.setPixmap(QPixmap::fromImage(image));
    _lblImage.show();
  }
}

void MainWindow::on_pbRecognize_clicked()
{
  _strPathTemp=QFileDialog::getOpenFileName(this, tr("Open File"),"/Img/Etalone",tr("Image files(*.*)"));
  if(!_strPathTemp.isEmpty())
  {
      QImage image(_strPathTemp);
      image = image.scaled(32,32);
      _lblImage.setPixmap(QPixmap::fromImage(image));
      _lblImage.show();

    int count=0;
    pict = QImage(_strPathTemp).scaled(ciWidth, ciHeight);
    for(unsigned char sign = chSignStart ; sign<= chSignFinish ;sign++)
    {
        fromPinkyToBrain[count]->setImageRef(pict);
        emit brain[count]->operateRecogn();
        count++;
    }
  }
  _strPathTemp="";
}

void MainWindow::on_pbTeach_clicked()
{
    bool bRunningThreadNOTDetected(true);

    if(zipper->getZipperState())
       bRunningThreadNOTDetected = false;
    for(int i=0; i< ciCountRepeats && bRunningThreadNOTDetected; ++i){
        if(fromPinkyToBrain[i]->getNeironState()!=enIdling)
            bRunningThreadNOTDetected = false;
    }
    if(!bRunningThreadNOTDetected){
        QMessageBox::warning(this, tr("Ошибка"),
                                         tr("Обучение уже запущено!\n"),
                                         QMessageBox::Ok );
        return;
    }
    _iCalcReady = 0;
    _iGoodTry =0 ;
    _iNDSTry = 0;
    _iDollarTry = 0;

    QString path;
    path = ui->lePath->text();
    QDir dataDir(path);  
    if(!(ui->chbRememberPath->isChecked()) || !(QDir(path).exists()))
    {
      path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                              "/Img",
                                                              QFileDialog::ShowDirsOnly
                                                              | QFileDialog::DontResolveSymlinks);
      dataDir.setPath(path);
      if(!dataDir.exists())
      {
        qDebug()<< "Dir doesn't exist";
        return;
      }
      ui->lePath->setText(path);
    }
        char upLimitSign( chSignFinish ), downLimitSign( chSignStart );
        if(ui->chbStudyNeiron->isChecked()){
          _upLimitNeiron =   ui->cbNeiron->currentText().toStdString().c_str()[0];
          _downLimitNeiron = ui->cbNeiron->currentText().toStdString().c_str()[0];
        }
        else
        {
          _upLimitNeiron =   chSignFinish;
          _downLimitNeiron =chSignStart;
        }
        if(ui->chbStudySign->isChecked()){
          upLimitSign =   ui->cbSign->currentText().toStdString().c_str()[0];
          downLimitSign = ui->cbSign->currentText().toStdString().c_str()[0];
        }

        ui->pbPicts->setValue(0);
        zipper->setStudyDataPtr(_pFullDataList);
        zipper->setPath(dataDir.absolutePath());
        emit sigZipFile(upLimitSign, downLimitSign);
        qDebug() <<"Zip called" << endl;
}

void MainWindow::on_pbSave_clicked()
{
   if(saveData!=nullptr)
       saveData->open();
}

void MainWindow::vSave(char sign,QString dirName, QString fileName)
{
  QString rrr(qApp->applicationDirPath());   //+fileName+".png");
  QDir dir(rrr);
  dir.cd("./..");
  dir.mkpath("./Weights/"+dirName);
  dir.cd("./Weights/"+dirName);
  qDebug() << (fromPinkyToBrain[sign-chSignStart]->savePictToFile(dir.absoluteFilePath(fileName+".png")) ? "save ok" : "failed");
}

void MainWindow::on_pushButton_clicked()
{
 emit sigStopCalc();
}

void MainWindow::sltSavePicts(QChar sign,QString& path)
{
    char chSign=sign.toLatin1();
    if(sign!=QChar(0))
    {
        fromPinkyToBrain[chSign-chSignStart]->savePictToFile(path);
    }
    else
    {
        foreach(neuron*t,fromPinkyToBrain)
            t->savePictToFile(path);
    }

}

void MainWindow::sltSaveWeights(QChar sign,QString& path)
{
    char chSign=sign.toLatin1();
    if(sign!=QChar(0))
    {
        fromPinkyToBrain[chSign-chSignStart]->saveWeightFile(path);
    }
    else
    {
        foreach(neuron*t,fromPinkyToBrain)
            t->saveWeightFile(path);
    }
}

void MainWindow::sltTeaching(bool correct)
{
  if(correct)
  {
    char sign;
    int cnt(_pFullDataList->size());
    ui->pbStudy_Rec->setMaximum(cnt);
    ui->pbStudy_Rec->setValue(0);
    QTime time(QTime::currentTime());
    qDebug() <<"Start: "<< time.toString()<<endl;

    for(sign = _downLimitNeiron; sign<=_upLimitNeiron; sign++)      //study
    {
          int currentSign = sign - chSignStart;
          fromPinkyToBrain[currentSign]->setStudyDataPtr(_pFullDataList);
          emit brain[currentSign]->operateStudy(ui->spbRounds->value());
    }
  }
  else
  {
    qDebug() <<" something is going wrong!" <<endl;
  }
}

void MainWindow::sltUpdateZip(int pr)
{
  if(pr)
    ui->pbPicts->setValue(pr);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /*QSettings settings("MyCompany", "MyApp");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());*/
  /*  foreach(Controller*t,brain)
        t->stopE();*/
    QMainWindow::closeEvent(event);
}

void MainWindow::sltAccumRes()
{
    ++_iCalcReady;
    if(_iCalcReady == ciCountRepeats)
    {
        _iCalcReady = 0;
        QMessageBox::information(this, tr("Оповещение"),
                                         tr("Обучение завершено!\n"),
                                         QMessageBox::Ok );
    }
}
