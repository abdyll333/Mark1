#include "neuron.h"
#include <cstdlib>
#include <ctime>
#include <QImage>
#include <QDebug>
#include <cmath>
#include <windows.h>
#include <omp.h>
#include <QFile>
#include <MarkConst.h>

const long clThreshold = 20000000;     //Порог превышения R функции нейрона

neuron::neuron(char sign, QObject *parent):
  QObject(parent),
  neuronSign(sign),
  _llThresholdSum(0),
  _iRunCnt(0),
  _iGood(0),
  _iNDS(0),
  _iDollar(0),
  _bStudied(false),
  _opImage(nullptr),
  _enState(enIdling)
{
    int nProcs=omp_get_max_threads();
    omp_set_num_threads(nProcs);
    _ucVectSensors.resize(ciWidth * ciHeight);
    _iVectWeight.resize(ciWidth * ciHeight);
    loadWeightsFile();
}

neuron::~neuron()
{
  _ucVectSensors.clear();
   _iVectWeight.clear();
   if(_opImage)
     delete _opImage;
}

bool neuron::checkValidSigm()
{
    long lSigmActiveLayer = calcTresholdValue();
    if(lSigmActiveLayer>=clThreshold)
    {
        //qDebug()<<"Эта буква определена как "<<neuronSign<<"!"<<endl;
        return true;
    }
    else {
        //qDebug()<<"Эта буква не определена как "<<neuronSign<<"!"<<endl;
    }
    return false;
}

long neuron::calcTresholdValue()
{
    long lTreshold=0;
    if(!_opImage)
    {
       qDebug()<<"Kosyak"<<endl;
    }
    else
    {
      quint8 const* line = _opImage->scanLine(0);
      int stride = _opImage->bytesPerLine();
      int index=0;
      for(int y = 0; y<ciHeight; y++, line += stride)
      {
          quint8 const* pix=line;
          for(int x=0; x < ciWidth; x++, pix += 4, index = y * ciWidth + x ){
              if((pix[0]<0xff) && (pix[1]<0xff) && (pix[2]<0xff))
              {
                  _ucVectSensors[ index ]=1;
                  lTreshold+=_iVectWeight[ index ];
              }
              else {
                  _ucVectSensors[ index ]=0;
              }
          }
      }
      _opImage = nullptr;
    }
    return lTreshold;
}



void neuron::setImageRef(const QImage &imgRef)
{
  _opImage = &(imgRef);
}

long neuron::getThreshold()
{
    return clThreshold;
}

void neuron::setWeightAR()
{
    srand(time_t(nullptr));
    int size=ciWidth * ciHeight;

    #pragma omp parallel for
    for(int i=0;i<size;i++)
    {
        _iVectWeight[i]=1+static_cast<int>(sqrt((rand()+rand()+rand())/3.)*rand())%100;
    }

}

void neuron::study(char sign)
{
    ++_iRunCnt;
    bool cal=checkValidSigm();
    if(sign!=neuronSign && cal)
    {
        sancionsOurKrimea(false);
        ++_iDollar;
        _bStudied = false  ;
    }
    else if(sign==neuronSign && !cal)
    {
       sancionsOurKrimea(true);
       ++_iNDS;
       _bStudied = false ;
    }
    else
    {
       ++_iGood;
    }
}

void neuron::sancionsOurKrimea(bool increase)
{
    int size = _iVectWeight.size(), delta=increase?1:-1;
    for(int i=0;i<size;i++)
       if(_ucVectSensors[i])
            _iVectWeight[i]+=delta;
}

bool neuron::savePictToFile(QString  path)
{
    QImage *image(new QImage(ciWidth, ciHeight , QImage::Format_RGB32));
    image->fill(Qt::white);
  int i, j, max(_iVectWeight[0]), sz(ciWidth * ciHeight);
  for(i=1; i < sz; ++i )
    if(max < _iVectWeight[i])
      max = _iVectWeight[i];

  qDebug() << " max weight " << max;
   //#pragma omp parallel for
    for( i = 0; i < ciWidth; ++i)
      for( j = 0; j <  ciHeight; ++j)
    {
        int clr = static_cast<int>(255 -  _iVectWeight[ j * ciWidth + i ] * 255.0 / max);
        if(clr>255)
            clr=255;
        image->setPixel(i ,j ,QColor(clr,clr,clr).rgb());
    }
    if(path.contains(".png"))
        return image->save(path);
    else
        return image->save(path+"/"+neuronSign+".png");
}

void neuron::saveWeightFile(QString filePath)
{
    QString strBuff;

    for(int i=0;i<_iVectWeight.size();i++)
    {
        if(i%ciWidth==0 && i>0)
           strBuff.append("\n");
        strBuff.append(QString::number(_iVectWeight[i]));
        strBuff.append(";");
    }
    QString filename=filePath+"/"+neuronSign+".csv";
    QFile fileWeight(filename);
    if (!fileWeight.open(QFile::WriteOnly))
    {
        qDebug("Error opening file");
    }
    else
    {
        fileWeight.write(strBuff.toStdString().c_str(), strBuff.length());
        fileWeight.close();
    }
    return;

}

void neuron::loadWeightsFile()
{
    QFile file("./Data/Weights/"+ QString(neuronSign)+".csv");
          if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
          {
              setWeightAR();
              return;
          }
          QTextStream in(&file);
          QString line;
          QStringList list;
          while (!in.atEnd())  //todo  странно
          {
              line=in.readAll();
          }
          line.replace("\n",";");
          list=(line.split(';', QString::SkipEmptyParts));

          for(int i=0;i<_iVectWeight.size();i++)
              _iVectWeight[i]=list[i].toInt();
}

void neuron::doStudy(const int roundCnt)
{
  if(_enState != enIdling){   //проверка на то, занят ли поток или в процессе остановки
     emit threadIsStopping(static_cast<int>(_enState));
     return;
  }
  else
     _enState = enWorked;    // если все ок, то арбайтен

  _iGood = _iNDS = _iDollar = 0;
  if(_pStudyData){
   int i,j, fileCnt(_pStudyData->count());
   //QString printInfo("Neiron %1 { Good: %2 ;   No itself symb(err): %3 ;   Itself symb(err): %4   }");
   for(i = 0;( i < roundCnt /*|| !_bStudied*/ ) && (_enState == enWorked); ++i){
      _bStudied = true;
      for(j = 0; (j < fileCnt ) && (_enState == enWorked); ++j){
          setImageRef(_pStudyData->at(j).image);
          study(_pStudyData->at(j).sign);
      }
   //qDebug()<<printInfo.arg(neuronSign).arg(_iGood,5).arg(_iNDS,5).arg(_iDollar,5)<<endl;
   }
   if(_enState == enStopped)
   {
     _enState = enIdling;
     emit resultStopped();     //извещение о принудительном завершении
   }
   else
   {
       _enState = enIdling;    //арбайтен завершен
       emit resultReady();
   }
  }
}

 void neuron::setStudyDataPtr(const dataPtr ptr)
 {
   if(_enState == enIdling)
     _pStudyData = ptr;
 }

 void neuron::doRecogn()
 {
   //qDebug() << neuronSign << " got it"<<endl;
     long lSigmActiveLayer = calcTresholdValue();
   //  ++_iRunCnt;
     if(lSigmActiveLayer>=clThreshold)
       emit recognReady(true, lSigmActiveLayer);
     else
       emit recognReady(false, lSigmActiveLayer);
 }

 void neuron::stopCalc()
 {
   if(_enState == enWorked)
   _enState = enStopped;
 }

