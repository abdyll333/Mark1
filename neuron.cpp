#include "neuron.h"
#include <cstdlib>
#include <ctime>
#include <QImage>
#include <QDebug>
#include <cmath>
#include <windows.h>
#include <omp.h>





const QPoint cqpSizeInputPicture=QPoint(1200,900);


neuron::neuron(char sign):
    neuronSign(sign),
    _lThreshold(3000000)
{
    int nProcs=omp_get_max_threads();
    omp_set_num_threads(nProcs);
    _ucVectSensors.resize(1200*900);
    _iVectWeight.resize(1200*900);
    setWeightAR();

}





bool neuron::checkValidSigm()
{
    long lSigmActiveLayer = calcTresholdValue();
    //qDebug()<<"Summ:"<<lSigmActiveLayer<<endl;
    //qDebug()<<"My neron sign:"<<static_cast<char>(neuronSign);
    if(lSigmActiveLayer>=_lThreshold)
    {
        qDebug()<<"Это буква :"<<neuronSign<<endl;
        return true;

    }
    return false;
}



long neuron::calcTresholdValue()
{

    long lTreshold=0;

    QImage image(_pathToFile);


    if(image.load(_pathToFile) && (image.width()==cqpSizeInputPicture.x() && image.height()==cqpSizeInputPicture.y()))
    {

        //image = image.convertToFormat(QImage::Format_Mono);

        quint8 const* line =image.scanLine(0);
        int stride=image.bytesPerLine();
        int height=image.height();
        int width=image.width();
        for(int y=0;y<height;y++,line+=stride)
        {
            quint8 const* pix=line;
            for(int x=0;x<width;x++,pix+=4){
                if((pix[0]!=0xff) && (pix[1]!=0xff) && (pix[2]!=0xff))
                {
                    _ucVectSensors[y*width+x]=1;
                    lTreshold+=_iVectWeight[y*width+x];
                }
                else {
                    _ucVectSensors[y*width+x]=0;
                }

            }
        }


        //qDebug()<<"Weight summ"<<lTreshold<<endl;
    }
    else
        qDebug()<<"Kosyak"<<endl;

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


void neuron::setWeightAR()
{
    srand(time_t(nullptr));
   // Sleep(neuronSign/10+5);
    int size=1200*900;
    #pragma omp parallel for
    for(int i=0;i<size;i++)
    {         
        _iVectWeight[i]=1+rand()%35;
    }

}



void neuron::study(char sign)
{    
    bool repeatSt;
  //  do{
        bool cal=checkValidSigm();
        repeatSt = false;
        if(sign!=neuronSign && cal)
        {
            sancionsOurKrimea(false);
            qDebug()<<"Dollar x2"<<endl;
            repeatSt = true;
        }
        else if(sign==neuronSign && !cal)
        {
           sancionsOurKrimea(true);
           qDebug()<<"NDS x2"<<endl;
           repeatSt = true;
        }
        else
        {
           qDebug()<<"GOOOOOOOOOOOOOD"<<endl;
        }
   // }
 //   while(repeatSt);
}


void neuron::sancionsOurKrimea(bool increase)
{
    int size=_iVectWeight.size(),delta=increase?5:-5;
    for(int i=0;i<size;i++)
       if(_ucVectSensors[i])
            _iVectWeight[i]+=delta;
}
