#ifndef NEURON_H
#define NEURON_H
#include <QVector>
#include <QObject>
#include "MarkConst.h"



class neuron : public QObject
{
  Q_OBJECT

    char neuronSign;
    QVector <unsigned char> _ucVectSensors;
    QVector <int> _iVectWeight;
    QString _pathToFile;
    long long _llThresholdSum;
    int            _iRunCnt;
    int            _iGood;
    int            _iNDS;
    int            _iDollar;
    bool         _bStudied;
    dataPtr    _pStudyData;
    const QImage*  _opImage;
    enThreadStates _enState;
    void setWeightAR();
    void sancionsOurKrimea(bool increase);

public slots:
    void doStudy(const int roundCnt);
    void doRecogn();
    void stopCalc();

signals:
    void resultReady();
    void resultStopped();
    void threadIsStopping(int);
    void recognReady( bool , long);

public:
    neuron(char, QObject*parent = nullptr);
    ~neuron();

    bool checkValidSigm();
    long calcTresholdValue();
    //void setPathToFile(QString);
    void setImageRef(const QImage &);
    long getThreshold();
    bool savePictToFile(QString );
    void saveWeightFile(QString filePath);
    void loadWeightsFile();
    int  getRunCount() const { return _iRunCnt; }
    enThreadStates getNeironState() const {return _enState; }

    void setStudyDataPtr(const dataPtr ptr);

    inline int  getLastStudyGoodCnt() const {return _iGood;}
    inline int  getLastStudyDollarCnt() const{return _iDollar;}     // ошибочно распознал символ как свой
    inline int  getLastStudyNDSCnt() const  {return _iNDS;}         // не распознал свой символ

    void study(char);
    inline char getNeuronSign() const { return neuronSign; }

};

#endif // NEURON_H
