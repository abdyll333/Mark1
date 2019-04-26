#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <omp.h>
#include <QTime>

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
  workerThread.wait(100);
  workerThread.start();
}

void ZipPicture::sltMakeData(char signUp, char signDown) //todo чекать количество файлов на переполнение буфера
{
  qDebug() <<"Zip got it" << endl;
  QDir dataDir(_strWorkDirectoryPath);
  QStringList filters;
  filters << "*.png" << "*.jpg" << "*.bmp";
  dataDir.setNameFilters(filters);
  if(_pDataList && !_strWorkDirectoryPath.isEmpty())
  {
   _pDataList->clear();
   int cnt(0), size(0);

#pragma omp parallel for
for(int sign = signDown; sign <= signUp; sign++)       //full list of files
{
    QDir dir(dataDir);
    if(dir.cd(_strWorkDirectoryPath+'/'+sign)){
      size += dir.entryList(QDir::Files).size()  ;
    }
}

   float progr(100.f / size);

    #pragma omp parallel for
    for(int sign = signDown; sign <= signUp; sign++)       //full list of files
    {
        QDir dir(dataDir);
        if(dir.cd(_strWorkDirectoryPath+'/'+sign)){
          foreach (QString fileinf, dir.entryList(QDir::Files)) {
                   _pDataList->append( studyData (sign, dir.absoluteFilePath(fileinf) ) );
                   ++cnt;
               emit  updateProgress(cnt * progr+1);
          }
        }
    }
     qDebug() <<"files cnt" <<size <<endl;
    emit resReady(true);
  }
  else
    emit resReady(false);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
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
  zipThread.start();

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
    vViewImage();
    int count=0;
    pict = QImage(_strPathTemp).scaled(ciWidth, ciHeight);
    for(unsigned char sign = chSignStart ; sign<= chSignFinish ;sign++)
    {
        fromPinkyToBrain[count]->setImageRef(pict);
        emit brain[count]->operateRecogn();
        /*if(fromPinkyToBrain[count]->checkValidSigm())   //check this
            qDebug()<<"Это значение похоже на:"<<static_cast<char>(sign)<<endl;*/
        count++;
    }
    //_strPathTemp=nullptr;
}

void MainWindow::on_pbTeach_clicked()
{
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

     /*   QStringList filters;
        filters << "*.png" << "*.jpg" << "*.bmp";
        dataDir.setNameFilters(filters);
         _listPathFiles.clear();*/
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
        zipper->setStudyDataPtr(&_listPathFiles);
        zipper->setPath(dataDir.absolutePath());
        emit sigZipFile(upLimitSign, downLimitSign);
        qDebug() <<"Zip called" << endl;
    /*    #pragma omp parallel for
        for(int signn = downLimitSign; signn <= upLimitSign; signn++)       //full list of files
        {
            QDir dir(dataDir);
            if(dir.cd(path+'/'+signn)){
              foreach (QString fileinf, dir.entryList(QDir::Files)) {
                       _listPathFiles << studyData(signn, dir.absoluteFilePath(fileinf));
              }

            }
        }*/
        /////////////
       /* int cnt(_listPathFiles.size());
        ui->pbPicts->setMaximum(cnt);
        ui->pbStudy_Rec->setMaximum(cnt);
        ui->pbPicts->setValue(0);
        ui->pbStudy_Rec->setValue(0);
        QTime time(QTime::currentTime());
        qDebug() <<"Start: "<< time.toString()<<endl;

          for(sign = downLimitNeiron; sign<=upLimitNeiron; sign++)      //study
          {
              int currentSign = sign - chSignStart;
              fromPinkyToBrain[currentSign]->setStudyDataPtr(&_listPathFiles);
              emit brain[currentSign]->operateStudy(ui->spbRounds->value());
        }*/
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
    int cnt(_listPathFiles.size());
    ui->pbStudy_Rec->setMaximum(cnt);
    ui->pbStudy_Rec->setValue(0);
    QTime time(QTime::currentTime());
    qDebug() <<"Start: "<< time.toString()<<endl;

      for(sign = _downLimitNeiron; sign<=_upLimitNeiron; sign++)      //study
      {
          int currentSign = sign - chSignStart;
          fromPinkyToBrain[currentSign]->setStudyDataPtr(&_listPathFiles);
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
