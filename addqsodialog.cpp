#include "addqsodialog.h"
#include "ui_addqsodialog.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

AddQSODialog::AddQSODialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddQSODialog)
{
    ui->setupUi(this);

    QDateTime currentDateTime(QDateTime::currentDateTimeUtc());
    ui->dateLineEdit->setText(currentDateTime.date().toString("yyyyMMdd"));
    ui->utcLineEdit->setText(currentDateTime.time().toString("hhmmss"));

    ui->callLineEdit->setFocus();
}

AddQSODialog::~AddQSODialog()
{
    delete ui;
}

// Main Window push "Add QSO" Button
void AddQSODialog::receiveUserDataFromAddQso(QString myCallsign, QString callsign)
{
    currentQSOId = -1;
    currentCall = myCallsign;
    ui->callLineEdit->setText(callsign);
    ui->recvQSLPushButton->setEnabled(false);
    ui->sendQSLPushButton->setEnabled(false);
    ui->deletePushButton->setEnabled(false);
    ui->pinnedCheckBox->setEnabled(true);

    ui->addQSOPushButton->setText("Add QSO");
}

// Main Window double click table widget
void AddQSODialog::reveiveQsoDataFromMain(QString myCallsign, QStringList list)
{
    currentQSOId = list[0].toInt();
    currentCall = myCallsign;
    ui->callLineEdit->setText(list[3]);
    ui->dateLineEdit->setText(list[1]);
    ui->utcLineEdit->setText(list[2]);
    ui->bandComboBox->setCurrentText(list[4]);
    ui->freqLineEdit->setText(list[5]);
    ui->modeComboBox->setCurrentText(list[6]);
    ui->RSTRcvdLineEdit->setText(list[7]);
    ui->RSTSentLineEdit->setText(list[8]);
    ui->contentTextEdit->setText(list[9]);
    ui->noteTextEdit->setText(list[12]);

    ui->recvQSLPushButton->setEnabled(true);
    ui->sendQSLPushButton->setEnabled(true);
    ui->deletePushButton->setEnabled(true);

    ui->pinnedCheckBox->setEnabled(false);

    ui->addQSOPushButton->setText("Modify QSO");
}

void AddQSODialog::on_calcelPushButton_clicked()
{
    this->close();
}


void AddQSODialog::on_addQSOPushButton_clicked()
{
    if(ui->callLineEdit->text().isEmpty())QMessageBox::information(this,"QSL Manager","Callsign cannot be Empty!",QMessageBox::Ok);
    else if(ui->dateLineEdit->text().isEmpty())QMessageBox::information(this,"QSL Manager","QSO Date cannot be Empty!",QMessageBox::Ok);
    else if(ui->utcLineEdit->text().isEmpty())QMessageBox::information(this,"QSL Manager","QSO Time cannot be Empty!",QMessageBox::Ok);
    else if(ui->bandComboBox->currentText().isEmpty())QMessageBox::information(this,"QSL Manager","Band cannot be Empty!",QMessageBox::Ok);
    else if(ui->modeComboBox->currentText().isEmpty())QMessageBox::information(this,"QSL Manager","Mode cannot be Empty!",QMessageBox::Ok);
    else
    {
        bool aaa = false;
        QSqlDatabase db;

        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(QString("log_%1.db").arg(currentCall));
        aaa = db.open();
        if (!aaa)qDebug() << "Error: Failed to connect database." << db.lastError();
        QSqlQuery sq(db);
        QString add_qso;
        if(currentQSOId < 0)// add new qso
        {
            add_qso = QString("insert into log("
                              "date, utc, call, band, freq, mode, "
                              "rst_r, rst_s, summary, note) values("
                              "%1, %2, '%3', '%4', %5, '%6',"
                              "'%7', '%8', '%9', '%10')")
                          .arg(ui->dateLineEdit->text().toInt())
                          .arg(ui->utcLineEdit->text().toInt())
                          .arg(ui->callLineEdit->text())
                          .arg(ui->bandComboBox->currentText())
                          .arg(ui->freqLineEdit->text().toDouble())
                          .arg(ui->modeComboBox->currentText())
                          .arg(ui->RSTRcvdLineEdit->text())
                          .arg(ui->RSTSentLineEdit->text())
                          .arg(ui->contentTextEdit->toPlainText())
                          .arg(ui->noteTextEdit->toPlainText());
        }
        else//modify current qso
        {
            add_qso = QString("update log set "
                              "date = %1, utc = %2, call = '%3', "
                              "band = '%4', freq = %5, mode = '%6', "
                              "rst_r = '%7', rst_s = '%8', "
                              "summary = '%9', note = '%10' where id = %11")
                          .arg(ui->dateLineEdit->text().toInt())
                          .arg(ui->utcLineEdit->text().toInt())
                          .arg(ui->callLineEdit->text())
                          .arg(ui->bandComboBox->currentText())
                          .arg(ui->freqLineEdit->text().toDouble())
                          .arg(ui->modeComboBox->currentText())
                          .arg(ui->RSTRcvdLineEdit->text())
                          .arg(ui->RSTSentLineEdit->text())
                          .arg(ui->contentTextEdit->toPlainText())
                          .arg(ui->noteTextEdit->toPlainText())
                          .arg(currentQSOId);
        }
        sq.prepare(add_qso);
        aaa = sq.exec();
        if (!aaa)qDebug() << add_qso << "\n" << "Error: Failed to update." << sq.lastError();

        db.close();
        emit sendRefresh();

        if(ui->pinnedCheckBox->isChecked()==false)
        {
            this->close();
        }
        else
        {
            ui->callLineEdit->setFocus();
        }
    }
}


void AddQSODialog::on_modeComboBox_currentTextChanged(const QString &arg1)
{
    if(arg1 == "CW")
    {
        ui->RSTRcvdLineEdit->setText("599");
        ui->RSTSentLineEdit->setText("599");
    }
    else if(arg1 == "AM" || arg1 == "FM" || arg1 == "SSB" )
    {
        ui->RSTRcvdLineEdit->setText("59");
        ui->RSTSentLineEdit->setText("59");
    }
    else if(arg1 == "SSTV")
    {
        ui->RSTRcvdLineEdit->setText("595");
        ui->RSTSentLineEdit->setText("595");
    }
    else
    {
        ui->RSTRcvdLineEdit->setText("0");
        ui->RSTSentLineEdit->setText("0");
    }
}


void AddQSODialog::on_deletePushButton_clicked()
{
    if(currentQSOId >= 0)
    {
        bool aaa = false;
        QSqlDatabase db;

        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(QString("log_%1.db").arg(currentCall));
        aaa = db.open();
        if (!aaa)qDebug() << "Error: Failed to connect database." << db.lastError();
        QSqlQuery sq(db);
        QString add_qso;

        add_qso = QString("delete from log where id = %1").arg(currentQSOId);

        sq.prepare(add_qso);
        aaa = sq.exec();
        db.close();
        emit sendRefresh();
        this->close();
    }
}

