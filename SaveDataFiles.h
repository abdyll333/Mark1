#ifndef SAVEDATAFILES_H
#define SAVEDATAFILES_H
#include <QDialog>

namespace Ui {
class Dialog;
}

class SaveDataFiles : public QDialog
{
    Q_OBJECT

 Ui::Dialog *ui;
public:
    explicit SaveDataFiles(QWidget *parent = nullptr);
    ~SaveDataFiles();


signals:
 void sigSavePict(QChar,QString&);
 void sigSaveWeight(QChar,QString&);

private slots:
 void on_pbBrowse_clicked();
 void on_pvSavePict_clicked();
 void on_pbSaveWeights_clicked();
};

#endif // SAVEDATAFILES_H
