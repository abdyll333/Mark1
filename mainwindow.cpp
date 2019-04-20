#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <omp.h>
#include <QTime>


const int ciAlphabet = 26;
const QChar chStart='a';
const QChar chFinish='z';
const int ciCountRepeats = chFinish.unicode() - chStart.unicode();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _iGoodTry(0),
    _iNDSTry(0),
    _iDollarTry(0)
{
    ui->setupUi(this);
    setInitNetValue();
    qDebug()<<omp_get_max_threads()<<endl;
    int nProcs=omp_get_max_threads();
    omp_set_num_threads(nProcs);
    qDebug()<<omp_get_num_threads()<<endl;

    for(int i = 0 ; i < ciCountRepeats ; ++i)  ////todo переделать под многоязычность
    {
      ui->cbNeiron->insertItem( 0, QString(QChar(chStart.unicode()+i)));
      ui->cbSign->insertItem( 0, QString(QChar(chStart.unicode()+i)));
    }

    ui->cbNeiron->setCurrentIndex(0);
    ui->cbSign->setCurrentIndex(0);
    ui->pbGood->setValue(0);
    ui->pbNDS->setValue(0);
    ui->pbDollar->setValue(0);
    ui->pbGood->setFormat("%v");
    ui->pbNDS->setFormat("%v");
    ui->pbDollar->setFormat("%v");

    ui->spbRounds->setMinimum(1);
    ui->spbRounds->setMaximum(100);
    ui->spbRounds->setValue(1);

    ui->chbRememberPath->setChecked(true);

    _Timer.setSingleShot(false);
    _Timer.setInterval(1000);                 //todo qthread helloooyyy

    connect(&_Timer, &QTimer::timeout, this, [=] () {
      ui->pbGood->setValue(_iGoodTry % ui->pbGood->maximum());
      ui->pbNDS->setValue(_iNDSTry % ui->pbNDS->maximum());
      ui->pbDollar->setValue(_iDollarTry % ui->pbDollar->maximum());
    });
  _Timer.start();
}

MainWindow::~MainWindow()
{
  foreach(neuron* n,fromPinkyToBrain)
    delete n;
  fromPinkyToBrain.clear();
  delete ui;
}

void MainWindow::on_pushButton_3_clicked()
{
    QApplication::quit();
}

void MainWindow::setInitNetValue()
{
    int count=0;
    for(char sign = chStart; sign <= chFinish;sign++)
    {
        fromPinkyToBrain.push_back(new neuron(sign));
        count++;
    }

}

void MainWindow::vViewImage()
{
  _strPathTemp=QFileDialog::getOpenFileName(this, tr("Open File"),"/Img/Etalone",tr("Image files(*.*)"));
  QImage image(_strPathTemp);
  image = image.scaled(32,32);
  _lblImage.setPixmap(QPixmap::fromImage(image));
  _lblImage.show();
}

void MainWindow::on_pbRecognize_clicked()
{
    vViewImage();
    int count=0;
    for(unsigned char sign = chStart ; sign<= chFinish ;sign++)
    {
        fromPinkyToBrain[count]->setPathToFile(static_cast<QString>(_strPathTemp));
        if(fromPinkyToBrain[count]->checkValidSigm())   //check this
            qDebug()<<"Это значение похоже на:"<<static_cast<char>(sign)<<endl;
        count++;
    }
    _strPathTemp=nullptr;
}

void MainWindow::on_pbTeach_clicked()
{
    _iGoodTry =0 ;
    _iNDSTry = 0;
    _iDollarTry = 0;
   // vViewImage();
    QFileDialog dialog(this);

    QString path;
    QDir dataDir(path);
    path = ui->lePath->text();
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

        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.bmp";
        dataDir.setNameFilters(filters);
         _listPathFiles.clear();
        QChar upLimitNeiron( chFinish ), downLimitNeiron( chStart ), upLimitSign( chFinish ), downLimitSign( chStart );
        QChar sign;
        if(ui->chbStudyNeiron->isChecked()){
          upLimitNeiron =      ui->cbNeiron->currentText().toStdString().c_str()[0];
          downLimitNeiron = ui->cbNeiron->currentText().toStdString().c_str()[0];
        }

        if(ui->chbStudySign->isChecked()){
          upLimitSign =      ui->cbSign->currentText().toStdString().c_str()[0];
          downLimitSign = ui->cbSign->currentText().toStdString().c_str()[0];
        }


        for(sign = downLimitSign; sign <= upLimitSign; sign++)       //full list of files
        {
            QDir dir(dataDir);
            if(dir.cd(path+'/'+sign)){
              foreach (QString fileinf, dir.entryList(QDir::Files)) {
                       _listPathFiles << studyData(sign, dir.absoluteFilePath(fileinf));
              }

            }
        }
        int cnt(_listPathFiles.size());
        ui->pbGood->setMaximum(cnt);
        ui->pbNDS->setMaximum(cnt);
        ui->pbDollar->setMaximum(cnt);
        ui->pbGood->setValue(0);
        ui->pbNDS->setValue(0);
        ui->pbDollar->setValue(0);
        //QString Path("Neiron%1_Rnd%2");

//#pragma omp parallel for private(_listPathFile) private(dataDir) //private()

        QTime time(QTime::currentTime());

        qDebug() <<"Start: "<< time.toString()<<endl;

        for(int a = 0; a < ui->spbRounds->value(); ++a){
          for(sign = downLimitNeiron; sign<=upLimitNeiron; sign++)      //study
          {
             // qDebug()<<"================== Neiron:"<<sign<<"========"<<endl;
              int currentSign = sign - chStart;
            //  #pragma omp parallel for private(fromPinkyToBrain)
              for(int i = 0; i < cnt; ++i)
              {
                  //qDebug()<<_listPathFile[i]<<endl;
                  fromPinkyToBrain[currentSign]->setPathToFile(static_cast<QString>(_listPathFiles[i].fileName));
                  switch(fromPinkyToBrain[currentSign]->study( _listPathFiles[i].sign )){
                    case -1:    _iDollarTry++;
                                    //emit(sigUpdtDollar());
                    break;
                    case -2:    _iNDSTry++;
                                   // emit(sigUpdtNDS());
                    break;
                    default:   _iGoodTry++;
                                   // emit(sigUpdtGood());
                  }
               }

             vSave(QChar(sign).toUpper(), QString::number(a+1));

             qDebug() <<"Time: "<< time.secsTo(QTime::currentTime())<<"secs; Avg sum: " << fromPinkyToBrain[currentSign]->getThresholdAvg()<<endl;
             time = QTime::currentTime();
             this->repaint();
          }
          qDebug() <<"Round  #" << a+1 <<":  Good " << _iGoodTry <<" ; NDS " << _iNDSTry << " ; Dollar" << _iDollarTry<<endl;
        }
       // _Timer.timeout();
      //  _Timer.stop();
}

void MainWindow::on_pbSave_clicked()
{
    QString rrr(qApp->applicationDirPath()+"/../");
    QFileDialog dialog(this);
   /* dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(qApp->applicationDirPath()+"/../");*/
    QString fileName(dialog.getSaveFileName(this, tr("Select file to save"),rrr,tr("Images (*.png )")));
    qDebug() << (fromPinkyToBrain[ui->cbNeiron->currentIndex()]->saveToFile(fileName) ? "save ok" : "failed");
}

void MainWindow::vSave(QString dirName, QString fileName)
{
  QString rrr(qApp->applicationDirPath()+"/../Weights/");   //+fileName+".png");
  QDir dir(rrr);
  dir.mkdir(dirName);
  dir.cd(dirName);
  qDebug() << (fromPinkyToBrain[ui->cbNeiron->currentIndex()]->saveToFile(dir.absoluteFilePath(fileName+".png")) ? "save ok" : "failed");
}

void MainWindow::on_pushButton_clicked()
{
    vViewImage();
}
