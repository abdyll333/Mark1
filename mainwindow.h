#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "neuron.h"
#include <QLabel>
#include <QTimer>

namespace Ui {
class MainWindow;
}

struct studyData
{
  char sign;
  QString fileName;
  studyData(char sn,QString path):
    sign(sn),
    fileName(path)
  { ; }
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

private:
    QString _strPathTemp;
    QVector <neuron *> fromPinkyToBrain;
    QList<studyData> _listPathFiles;
    QLabel _lblImage;
    int _iGoodTry;
    int _iNDSTry;
    int _iDollarTry;
    QTimer _Timer;

    void setInitNetValue();
    void vViewImage();
    void vSave(QString , QString );
    Ui::MainWindow *ui;

signals:
    void sigUpdtGood();
    void sigUpdtNDS();
    void sigUpdtDollar();
};

#endif // MAINWINDOW_H
