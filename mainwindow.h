#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "neuron.h"
#include <QLabel>

namespace Ui {
class MainWindow;
}

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

private:
    QString _strPathTemp;
    QVector <neuron *> fromPinkyToBrain;
    QStringList _listPathFile;
    QLabel _lblImage;
    void setInitNetValue();
    void vViewImage();
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
