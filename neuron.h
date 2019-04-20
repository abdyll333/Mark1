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
    void setWeightAR();
    void sancionsOurKrimea(bool increase);

public:
    neuron(char);
    bool checkValidSigm();
    long calcTresholdValue();
    void setPathToFile(QString);
    long getThreshold();


    void study(char);


};

#endif // NEURON_H
