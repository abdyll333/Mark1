#include "SaveDataFiles.h"
#include "ui_SaveDataFiles.h"
#include "MarkConst.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

SaveDataFiles::SaveDataFiles(QWidget *parent):
    QDialog(parent),
    ui(new Ui::Dialog)
{
   ui->setupUi(this);
   for(int i = 0 ; i <= ciCountRepeats ; ++i)  ////todo переделать под многоязычность
   {
     ui->cbSymbol->addItem(QString(QChar(chStart.unicode()+i)));
   }
   ui->cbSymbol->setCurrentIndex(0);


}

SaveDataFiles::~SaveDataFiles()
{
     delete ui;
}

void SaveDataFiles::on_pbBrowse_clicked()
{
    QString path;
    path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                            ".",
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
    ui->lineEdit->setText(path);



}

void SaveDataFiles::on_pvSavePict_clicked()
{
     QString path(ui->lineEdit->text());
     if((path.size()) && (QDir(path).exists()))
     {
         QDir(path).mkdir("PictStudy");
          path+="/PictStudy";
          QChar sign(ui->chbSaveOneSymbol->isChecked()?ui->cbSymbol->currentText()[0]:QChar(0));
          emit sigSavePict(sign,path);
     }
     else
         QMessageBox::warning(this, tr("Ошибка"),
                                          tr("Выбранная папка для сохранения не существует!\n"
                                             "Нажмите кнопку '...' для сохранения."),
                                          QMessageBox::Ok );

}

void SaveDataFiles::on_pbSaveWeights_clicked()
{
    QString path(ui->lineEdit->text());
    if((path.size()) && (QDir(path).exists()))
    {
        QDir(path).mkdir("Weights");
        path+="/Weights";
        QChar sign(ui->chbSaveOneSymbol->isChecked()?ui->cbSymbol->currentText()[0]:QChar(0));
        emit sigSaveWeight(sign,path);
    }
    else
        QMessageBox::warning(this, tr("Ошибка"),
                                         tr("Выбранная папка для сохранения не существует!\n"
                                            "Нажмите кнопку '...' для сохранения."),
                                         QMessageBox::Ok );
}
