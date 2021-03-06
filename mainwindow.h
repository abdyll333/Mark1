#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "SaveDataFiles.h"
#include "neuron.h"
#include <QLabel>
#include <QTimer>
#include <QThread>
#include "MarkConst.h"
#include <QDebug>

namespace Ui {
class MainWindow;
}

class ZipPicture : public QObject
{
  Q_OBJECT
  QString _strWorkDirectoryPath;
  dataPtr _pDataList;
  enThreadStates _enState;
public:
  ZipPicture(QObject * parent = nullptr):
    QObject(parent),
    _strWorkDirectoryPath(""),
    _pDataList(nullptr),
    _enState(enIdling)
  {;}

  void setPath(const QString &path) { if(_enState ==enIdling) _strWorkDirectoryPath = path ; }
  void setStudyDataPtr(const dataPtr ptr) { if(_enState ==enIdling) _pDataList = ptr; }
  dataPtr getPreparedData()const { return _pDataList;}
  enThreadStates getZipperState() const {return _enState; }
public slots:
  void sltMakeData(char, char);
signals:
  void resReady(bool);
  void updateProgress(int);

};

 class Controller : public QObject
 {
     Q_OBJECT
     QThread workerThread;
     neuron *worker;
 public:
     Controller(neuron* brain, char sign) {
         if(brain)
           worker = brain;
         else
           worker = new neuron(sign, parent());
         worker->moveToThread(&workerThread);
         connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
         connect(this, &Controller::operateStudy, worker, &neuron::doStudy);
         connect(this, &Controller::operateRecogn, worker, &neuron::doRecogn);
         connect(worker, &neuron::resultReady, this, &Controller::handleResults);
         connect(worker, &neuron::resultStopped, this, &Controller::stoppedHandle);
         connect(worker, &neuron::recognReady, this, &Controller::sltResultsRecogn);
         connect(worker, &neuron::threadIsStopping,
                 [&](int reason)
                    {
                        if(reason == enWorked)
                            qDebug() << tr("Поток ")<< worker->getNeuronSign()
                                   <<tr("работает. Подождите его полной остановки.") ;
                        else
                            qDebug() << tr("Поток ")<< worker->getNeuronSign()
                                   <<tr("останавливается. Подождите его полной остановки.") ;
                    });
         workerThread.start();
         workerThread.setPriority(QThread::LowestPriority);
     }

     ~Controller() {
         worker->stopCalc();
         workerThread.quit();
         workerThread.wait();
     }

 public slots:
     void handleResults();
     void stoppedHandle();
     void sltResultsRecogn( bool, long );
     void stopE();

 signals:
     void operateStudy(const int);
     void operateRecogn();
 };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_3_clicked();

    void on_pbRecognize_clicked();

    void on_pbTeach_clicked();

    void on_pbSave_clicked();

    void on_pushButton_clicked();

    void sltSavePicts(QChar,QString&);

    void sltSaveWeights(QChar,QString&);

    void sltTeaching(bool);

    void sltUpdateZip(int);

    void sltAccumRes();


private:
    Ui::MainWindow *ui;
    QString _strPathTemp;
    QVector <neuron *> fromPinkyToBrain;
    QVector <Controller *> brain;
    dataPtr _pFullDataList;
    QLabel _lblImage;
    int _iCalcReady;
    int _iGoodTry;
    int _iNDSTry;
    int _iDollarTry;
    char _upLimitNeiron;
    char _downLimitNeiron;
    SaveDataFiles* saveData;
    QImage pict;
    QThread zipThread;
    ZipPicture *zipper;

    void setInitNetValue();
    void vViewImage();
    void vSave(char sign, QString , QString );
    void closeEvent(QCloseEvent *);

signals:
    void sigUpdtGood();
    void sigUpdtNDS();
    void sigUpdtDollar();
    void sigStopCalc();
    void sigZipFile(char, char);


};

#endif // MAINWINDOW_H
