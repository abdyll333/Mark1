#ifndef NEURON_H
#define NEURON_H
#include <QVector>


class neuron
{

    char neuronSign;
    long _lThreshold;//Порог превышения R функции нейрона
    QVector <unsigned char> _ucVectSensors;
    QVector <int> _iVectWeight;
    QString _pathToFile;
    long long _llThresholdSum;
    int            _iRunCnt;
    void setWeightAR();
    void sancionsOurKrimea(bool increase);

public:
    neuron(char);
    bool checkValidSigm();
    long calcTresholdValue();
    void setPathToFile(QString);
    long getThreshold();
    long getThresholdAvg();
    bool saveToFile(QString );


    int study(char);


};

#endif // NEURON_H
