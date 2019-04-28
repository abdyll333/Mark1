#ifndef MARKCONST_H
#define MARKCONST_H
#include <QChar>
#include <QImage>


const QChar chStart='a';
const QChar chFinish='z';
const char chSignStart=chStart.toLatin1();
const char chSignFinish=chFinish.toLatin1();
const int ciCountRepeats = chFinish.unicode() - chStart.unicode();
const int ciWidth = 32;
const int ciHeight = 32;

enum enThreadStates{
  enIdling = 0,  //холостой ход
  enWorked,
  enStopped,
  enAllStates
};

struct studyData
{
  char sign;
  QString fileName;
  QImage image;
  studyData(char sn,QString path):
    sign(sn),
    fileName(path)
  {
    image = QImage(path).scaled(ciWidth,ciHeight) ;
  }
  studyData():
    sign(0),
    fileName("")
  {
  }
};

Q_DECLARE_METATYPE(studyData)

typedef  QList<studyData> data;
Q_DECLARE_METATYPE(data)
typedef  QList<studyData>* dataPtr;
Q_DECLARE_METATYPE(dataPtr)


#endif // MARKCONST_H
