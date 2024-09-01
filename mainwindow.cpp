#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "addqsodialog.h"
#include "logindialog.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

void MainWindow::receiveUserData(QString data)
{
    currentCall = data;
    //ui->textBrowser->setPlainText(data);
    ui->currentCallLabel->setText(data + "'s Logbook");
    refreshTable();
}

void MainWindow::receiveRefresh()
{
    refreshTable();
}

void MainWindow::refreshTable()
{
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);

    QStringList headerList;
    ui->tableWidget->setColumnCount(13);
    headerList << "id" << "QSO Date" << "UTC" << "Call Sign" << "Band" << "Freq.(MHz)" << "Mode" << "RST Rcvd" << "RST Sent"
               << "Content Summary" << "QSL Rcvd" << "QSL Sent" << "Note";
    ui->tableWidget->setHorizontalHeaderLabels(headerList);

    int columnWidth[13] = {40, 80, 60, 80, 40, 80, 40, 80, 80, 200, 80, 80, 200};
    for(int i = 0; i < 13; i++)
    {
        ui->tableWidget->setColumnWidth(i, columnWidth[i]);
    }

    bool aaa = false;
    QSqlDatabase db;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QString("log_%1.db").arg(currentCall));
    aaa = db.open();
    if (!aaa)qDebug() << "Error: Failed to connect database." << db.lastError();
    QSqlQuery sq(db);
    QString add_qso = QString("select * from log");
    // Filter condition
    int isWhere = 0;
    if(!ui->callLineEdit->text().isEmpty())
    {
        add_qso.append(isWhere++==0?" where ":" and ");
        add_qso.append(QString("call like '%%1%'").arg(ui->callLineEdit->text()));
    }
    if(!ui->date1LineEdit->text().isEmpty())
    {
        add_qso.append(isWhere++==0?" where ":" and ");
        add_qso.append(QString("date >= %1").arg(ui->date1LineEdit->text()));
    }
    if(!ui->date2LineEdit->text().isEmpty())
    {
        add_qso.append(isWhere++==0?" where ":" and ");
        add_qso.append(QString("date <= %1").arg(ui->date2LineEdit->text()));
    }
    if(!ui->bandComboBox->currentText().isEmpty() && ui->bandComboBox->currentText()!="All Bands")
    {
        add_qso.append(isWhere++==0?" where ":" and ");
        add_qso.append(QString("band == '%1'").arg(ui->bandComboBox->currentText()));
    }
    if(!ui->modeComboBox->currentText().isEmpty() && ui->modeComboBox->currentText()!="All Modes")
    {
        add_qso.append(isWhere++==0?" where ":" and ");
        add_qso.append(QString("mode == '%1'").arg(ui->modeComboBox->currentText()));
    }

    if(ui->sortOrderComboBox->currentText()=="QSO ID")
    {
        add_qso.append(" order by id ");
        add_qso.append(ui->descendCheckBox->isChecked()?"desc":"asc");
    }
    else if(ui->sortOrderComboBox->currentText()=="QSO Date Time")
    {
        add_qso.append(QString(" order by date %1, utc %1 ").arg(ui->descendCheckBox->isChecked()?"desc":"asc"));
    }
    else if(ui->sortOrderComboBox->currentText()=="Call Sign")
    {
        add_qso.append(" order by call ");
        add_qso.append(ui->descendCheckBox->isChecked()?"desc":"asc");
    }


    sq.prepare(add_qso);
    sq.exec();

    bool available = sq.first();
    while(available)
    {
        int RowCount;
        RowCount=ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(RowCount + 1);
        for(int i = 0; i < 13; i++)
        {
            QString str = sq.value(i).toString();
            if(i == 2)str = QString("%1").arg(sq.value(i).toInt(), 6, 10, QLatin1Char('0'));
            ui->tableWidget->setItem(RowCount, i, new QTableWidgetItem(str));
        }
        available = sq.next();
    }

    db.close();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addQSOPushButton_clicked()
{
    AddQSODialog *addqsodialog = new AddQSODialog;
    connect(this, SIGNAL(sendData2(QString,QString)), addqsodialog, SLOT(receiveUserDataFromAddQso(QString,QString)));
    connect(addqsodialog, SIGNAL(sendRefresh()), this, SLOT(receiveRefresh()));
    emit sendData2(currentCall, ui->callLineEdit->text());  //获取lineEdit的输入并且传递出去
    addqsodialog->show();
}


void MainWindow::on_toolButton_clicked()
{
    ui->callLineEdit->clear();
}


void MainWindow::on_tableWidget_cellDoubleClicked(int row, int column)
{
    AddQSODialog *addqsodialog = new AddQSODialog;
    connect(this, SIGNAL(sendData3(QString,QStringList)), addqsodialog, SLOT(reveiveQsoDataFromMain(QString,QStringList)));
    connect(addqsodialog, SIGNAL(sendRefresh()), this, SLOT(receiveRefresh()));
    QStringList list;
    for(int i = 0; i < 13; i++)
    {
        list << ui->tableWidget->item(row, i)->text();
    }
    emit sendData3(currentCall, list);  //获取lineEdit的输入并且传递出去
    addqsodialog->show();
}


void MainWindow::on_pushButton_2_clicked()
{
    refreshTable();
}


void MainWindow::on_pushButton_3_clicked()
{
    LoginDialog *logindialog = new LoginDialog();
    logindialog->show();
    this->close();
}


void MainWindow::on_pushButton_clicked()
{
    int found = 0, added = 0, dulp = 0;
    QString fileName = QFileDialog::getOpenFileName(this, "Import ADIF File", "", "ADIF File(*.adi *.adif);;All Files(*.*)");
    if(!fileName.isEmpty())
    {
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::information(this,"QSL Manager","Open ADIF File Error!",QMessageBox::Ok);
            return;
        }
        QTextStream fileStream(&file);
        //Find <EOH> for start
        while(!fileStream.atEnd())
        {
            char i = 0;
            fileStream >> i;

            QString buf;
            if(i == '<')
            {
                while(1)
                {
                    fileStream >> i;
                    if(i == '>')break;
                    buf.append(i);
                }
                if(buf.toLower()!="eoh")
                {
                    QMessageBox::information(this,"QSL Manager","ADIF File Format Error!",QMessageBox::Ok);
                    return;
                }
                break;
            }
        }
        //
        while(!fileStream.atEnd())
        {
            QString line_buf = fileStream.readLine();
            QTextStream lineStream(&line_buf);

            QVector<QPair<QString,QString>> vecBuf;
            QString keyBuf;
            while(!lineStream.atEnd())
            {
                char i = 0;
                lineStream >> i;

                if(i == '<')
                {
                    while(1)
                    {
                        lineStream >> i;
                        if(i == '>')break;
                        keyBuf.append(i);
                    }
                    if(keyBuf.toLower() == "eor")
                    {
                        found++;
                        //deal with vector<pair>, find if dulplicated, if not, send to database
                        QString date, utc, call, band, freq, mode, rst_r, rst_s;
                        for(int i = 0; i < vecBuf.length(); i++)
                        {
                            if(vecBuf[i].first.toLower() == "qso_date")date=vecBuf[i].second;
                            else if(vecBuf[i].first.toLower() == "time_on")utc=vecBuf[i].second;
                            else if(vecBuf[i].first.toLower() == "call")call=vecBuf[i].second;
                            else if(vecBuf[i].first.toLower() == "band")band=vecBuf[i].second;
                            else if(vecBuf[i].first.toLower() == "freq")freq=vecBuf[i].second;
                            else if(vecBuf[i].first.toLower() == "mode")mode=vecBuf[i].second;
                            else if(vecBuf[i].first.toLower() == "rst_sent")rst_s=vecBuf[i].second;
                            else if(vecBuf[i].first.toLower() == "rst_rcvd")rst_r=vecBuf[i].second;
                        }

                        bool aaa = false;
                        QSqlDatabase db;

                        db = QSqlDatabase::addDatabase("QSQLITE");
                        db.setDatabaseName(QString("log_%1.db").arg(currentCall));
                        aaa = db.open();
                        if (!aaa)qDebug() << "Error: Failed to connect database." << db.lastError();
                        QSqlQuery sq(db);
                        QString find_qso;
                        find_qso = QString("select * from log where "
                                           "date = %1 and utc = %2 "
                                           "and call = '%3' and band = '%4' "
                                           "and freq = %5 and mode = '%6'").arg(date, utc, call, band, freq, mode);
                        sq.prepare(find_qso);
                        aaa = sq.exec();
                        if (!aaa)qDebug() << find_qso << "\n" << "Error: Failed to update." << sq.lastError();
                        sq.first();
                        int size = sq.at() + 1;
                        if(size <= 0)
                        {
                            QString add_qso;
                            add_qso = QString("insert into log("
                                              "date, utc, call, band, freq, mode, "
                                              "rst_r, rst_s) values("
                                              "%1, %2, '%3', '%4', %5, '%6',"
                                              "'%7', '%8')").arg(date, utc, call, band, freq, mode, rst_r, rst_s);
                            sq.prepare(add_qso);
                            aaa = sq.exec();
                            if (!aaa)qDebug() << add_qso << "\n" << "Error: Failed to update." << sq.lastError();

                            added++;
                        }
                        else dulp++;
                        db.close();
                        break;
                    }
                    else
                    {
                        QStringList list = keyBuf.split(":");
                        if(list.length() <= 1 || list.length() > 2)
                        {
                            QMessageBox::information(this,"QSL Manager","Open ADIF File Error!",QMessageBox::Ok);
                            return;
                        }
                        QPair<QString, QString>pairBuf;
                        pairBuf.first = list[0];
                        for(int k = 0; k < list[1].toInt(); k++)
                        {
                            lineStream >> i;
                            pairBuf.second.append(i);
                        }
                        vecBuf.push_back(pairBuf);
                        keyBuf = "";
                    }
                }
            }

        }
        QMessageBox::information(this,"QSL Manager",QString("ADIF File Imported.\n"
                                                              "\t %1 QSO(s) Found,\n"
                                                              "\t %2 QSO(s) Added,\n"
                                                              "\t %3 QSO(s) Duplicated.").arg(found).arg(added).arg(dulp),QMessageBox::Ok);
        file.close();
        refreshTable();
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    QString strSaveName = QFileDialog::getSaveFileName(this,"Export to ADIF File","export_adif.adi","ADIF File(*.adi *.adif)");
    if(!strSaveName.isEmpty())
    {
        QFile file(strSaveName);
        if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::information(this,"QSL Manager","Open ADIF File Error!",QMessageBox::Ok);
            return;
        }
        QTextStream stream(&file);
        stream << "ADIF File Created by QSL Manager <eoh>\n";
        for(int i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            stream << QString("<qso_date:%1>").arg(ui->tableWidget->item(i,1)->text().length()) << ui->tableWidget->item(i,1)->text()
                << QString(" <time_on:%1>").arg(ui->tableWidget->item(i,2)->text().length()) << ui->tableWidget->item(i,2)->text()
                << QString(" <call:%1>").arg(ui->tableWidget->item(i,3)->text().length()) << ui->tableWidget->item(i,3)->text()
                << QString(" <band:%1>").arg(ui->tableWidget->item(i,4)->text().length()) << ui->tableWidget->item(i,4)->text()
                << QString(" <freq:%1>").arg(ui->tableWidget->item(i,5)->text().length()) << ui->tableWidget->item(i,5)->text()
                << QString(" <mode:%1>").arg(ui->tableWidget->item(i,6)->text().length()) << ui->tableWidget->item(i,6)->text()
                << QString(" <rst_rcvd:%1>").arg(ui->tableWidget->item(i,7)->text().length()) << ui->tableWidget->item(i,7)->text()
                << QString(" <rst_sent:%1>").arg(ui->tableWidget->item(i,8)->text().length()) << ui->tableWidget->item(i,8)->text()
                << " <eor>\n";
        }
        file.close();
    }
}

