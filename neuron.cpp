#include "neuron.h"
#include <cstdlib>
#include <ctime>
#include <QImage>
#include <QDebug>
#include <cmath>
#include <windows.h>
#include <omp.h>

const int ciWidth = 32;
const int ciHeight = 32;
const QPoint cqpSizeInputPicture=QPoint(ciWidth, ciHeight);


neuron::neuron(char sign):
    neuronSign(sign),
    _lThreshold(100000),
    _llThresholdSum(0),
    _iRunCnt(0)
{
    int nProcs=omp_get_max_threads();
    omp_set_num_threads(nProcs);
    _ucVectSensors.resize(ciWidth * ciHeight);
    _iVectWeight.resize(ciWidth * ciHeight);
    setWeightAR();
}

bool neuron::checkValidSigm()
{
    long lSigmActiveLayer = calcTresholdValue();
    _llThresholdSum += lSigmActiveLayer;
    ++_iRunCnt;
   // qDebug()<<"Summ:"<<lSigmActiveLayer;//<<endl;
    //qDebug()<<"My neron sign:"<<static_cast<char>(neuronSign);
    if(lSigmActiveLayer>=_lThreshold)
        return true;
    return false;
}

long neuron::calcTresholdValue()
{
    long lTreshold=0;
    QImage image;

    if(!image.load(_pathToFile))
    {
       qDebug()<<"Kosyak"<<endl;
    }
    else
    {
      image = image.scaled(32,32);
      quint8 const* line =image.scanLine(0);
      int stride=image.bytesPerLine();
      int index=0;
      for(int y = 0; y<ciHeight; y++, line += stride)
      {
          quint8 const* pix=line;
          for(int x=0; x < ciWidth; x++, pix += 4, index = y * ciWidth + x ){
              if((pix[0]!=0xff) && (pix[1]!=0xff) && (pix[2]!=0xff))
              {
                  _ucVectSensors[ index ]=1;
                  lTreshold+=_iVectWeight[ index ];
              }
              else {
                  _ucVectSensors[ index ]=0;
              }
          }
      }
    }
    return lTreshold;
}



void neuron::setPathToFile(QString path)
{
    _pathToFile=path;
}

long neuron::getThreshold()
{
    return _lThreshold;
}

long neuron::getThresholdAvg()
{
    return (_llThresholdSum*1.0)/_iRunCnt;
}


void neuron::setWeightAR()
{
    srand(time_t(nullptr));
    int size=ciWidth * ciHeight;

    #pragma omp parallel for
    for(int i=0;i<size;i++)
    {
        _iVectWeight[i]= 1+static_cast<int>(sqrt((rand()+rand()+rand())/3.)*rand())%100;
    }

}

int neuron::study(char sign)
{
    bool cal=checkValidSigm();
    if(sign!=neuronSign && cal)
    {
        sancionsOurKrimea(false);
        return -1;
    }
    else if(sign==neuronSign && !cal)
    {
       sancionsOurKrimea(true);
         return -2;
    }
    else
    {
       return 0;
    }
}


void neuron::sancionsOurKrimea(bool increase)
{
    int size = _iVectWeight.size(), delta=increase?1:-1;
    for(int i=0;i<size;i++)
       if(_ucVectSensors[i])
            _iVectWeight[i]+=delta;
}

bool neuron::saveToFile(QString  path)
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
        int clr = 255 -  _iVectWeight[ j * ciWidth + i ] * 255 / max;
        image->setPixel(i ,j ,QColor(clr,clr,clr).rgb());
    }

    return image->save(path);
}
