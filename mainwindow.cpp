#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <omp.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setInitNetValue();
    qDebug()<<omp_get_max_threads()<<endl;
    int nProcs=omp_get_max_threads();
    omp_set_num_threads(nProcs);
    qDebug()<<omp_get_num_threads()<<endl;
#pragma omp parallel for
    for(int i=0;i<10;i++)
    {
        int tid=omp_get_thread_num();
        qDebug()<<tid<<endl;
    }

}

MainWindow::~MainWindow()
{
    for(unsigned char sign='a';sign<='z';sign++)
    {
        neuron* n = fromPinkyToBrain.takeAt('z'-sign);
        delete n;
    }
    delete ui;

}

void MainWindow::on_pushButton_3_clicked()
{
    QApplication::quit();
}



void MainWindow::setInitNetValue()
{
   // _listPathFile=QFileDialog::getOpenFileNames(this, tr("Open File"),"/Img/Etalone",tr("Image files(*.*)"));

    int count=0;
    for(char sign='a';sign<='z';sign++)
    {
        fromPinkyToBrain.push_back(new neuron(sign));
       // fromPinkyToBrain[count]->setPathToFile(static_cast<QString>(_listPathFile[count]));
        count++;
    }

}



void MainWindow::vViewImage()
{
    _strPathTemp=nullptr;
  _strPathTemp=QFileDialog::getOpenFileName(this, tr("Open File"),"/Img/Etalone",tr("Image files(*.*)"));
  QImage image(_strPathTemp);
  _lblImage.setPixmap(QPixmap::fromImage(image));
  _lblImage.show();


}

void MainWindow::on_pbRecognize_clicked()
{
    vViewImage();

    int count=0;
    for(unsigned char sign='a';sign<='z';sign++)
    {
        fromPinkyToBrain[count]->setPathToFile(static_cast<QString>(_strPathTemp));
        if(fromPinkyToBrain[count]->checkValidSigm())   //check this
            qDebug()<<"Это значение похоже на:"<<static_cast<char>(sign)<<endl;
        count++;
    }
}


void MainWindow::on_pbTeach_clicked()
{
   // vViewImage();
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory("./Img");
    dialog.setLabelText(QFileDialog::LookIn, tr("Select dir"));
    QString path;
    //path=dialog.getOpenFileName(this,,);
    if (dialog.exec())
        _listPathFile = dialog.selectedFiles();
    if(!_listPathFile.size())
        return;
    path = _listPathFile[0];
    QDir dataDir(path);
    int count=0;
    if(dataDir.exists(path))
    {
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.bmp";
        dataDir.setNameFilters(filters);
        QDir dir(dataDir);
// #pragma omp parallel for private(_listPathFile) private(dataDir) private(count)
        for(char sign='a';sign<='z';sign++)
        {
            qDebug()<<"================== Sign:"<<sign<<"========"<<endl;

            if(dataDir.cd(path+'/'+sign))
            {
               _listPathFile = dataDir.entryList(QDir::Files);
               for(int i=0;i<_listPathFile.size();++i)
               {
                   qDebug()<<_listPathFile[i]<<endl;
                    for(char jojo='a';jojo<='b';jojo++)
                    {
                       count = jojo -'a';
                       fromPinkyToBrain[count]->setPathToFile(dataDir.absoluteFilePath(static_cast<QString>(_listPathFile[i])));
                       fromPinkyToBrain[count]->study(jojo);
                    }

               }
            }
            //count++;

        }
    }
}
