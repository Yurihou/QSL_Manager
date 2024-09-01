#include "logindialog.h"
#include "./ui_logindialog.h"

#include "mainwindow.h"
#include "regdialog.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>
#include <QMessageBox>

#include "stringxor.h"

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    bool aaa = false;
    QSqlDatabase db;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user_pass.db");
    aaa = db.open();
    if (!aaa)qDebug() << "Error: Failed to connect database." << db.lastError();
    QSqlQuery sq(db);
    QString look_for_new = "select name from sqlite_master where name = 'users'";
    sq.prepare(look_for_new);
    sq.exec();
    sq.last();
    int size = sq.at() + 1;
    if(size <= 0)
    {
        QString create_sql = "create table users ("
                             "id integer primary key autoincrement, "
                             "user text unique not null, "
                             "pass text not null, "
                             "phone integer not null,"
                             "level integer not null)";
        sq.prepare(create_sql);
        sq.exec();


        //create_sql = "insert into users(user, pass, phone, level) values('root', '8888', '114514', 0)";
        create_sql = QString("insert into users(user, pass, phone, level) values('root', '%1', '114514', 0)").arg(stringXOR("8888","root"));
        sq.prepare(create_sql);
        sq.exec();
    }
    else if(sq.size()==1)
    {
        qDebug() << "sql database is right.";
    }
    db.close();

    // Preference for last username auto filling
    QSettings settings("settings.ini",QSettings::IniFormat);
    settings.beginGroup("login");
    ui->userLineEdit->setText(settings.value("lastuser").toString());
    if(settings.value("rempass").toString() == "1")
    {
        ui->checkBox->setChecked(true);
        ui->passLineEdit->setText(stringXOR(settings.value("pass").toString(), settings.value("lastuser").toString()));
    }
    else ui->checkBox->setChecked(false);

    if(ui->userLineEdit->text().isEmpty())ui->userLineEdit->setFocus();
    else ui->passLineEdit->setFocus();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginPushButton_clicked()
{
    QSqlDatabase db;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user_pass.db");
    QSqlQuery sq(db);
    db.open();

    QString look_for_this = "select user, pass, level from users where user = '" + ui->userLineEdit->text() + "'";
    sq.prepare(look_for_this);
    sq.exec();
    sq.first();
    int size = sq.at() + 1;
    if(size <= 0)
    {
        QMessageBox::information(this,"QSL Manager","This User is not Registered yet.",QMessageBox::Ok);
        db.close();
    }
    else
    {
        QString pass = stringXOR(sq.value(1).toString(), ui->userLineEdit->text());
        int level = sq.value(2).toInt();
        if (pass != ui->passLineEdit->text())
        {
            QMessageBox::information(this,"QSL Manager","Wrong Username or Password!",QMessageBox::Ok);
            db.close();
        }
        else if(level == 0)
        {
            QMessageBox::information(this,"QSL Manager","This is root user.\n"
                                                          "Root user function has not developed yet.",QMessageBox::Ok);
            db.close();
        }
        else
        {
            MainWindow *mainwindow = new MainWindow;
            connect(this, SIGNAL(sendData(QString)), mainwindow, SLOT(receiveUserData(QString)));
            emit sendData(ui->userLineEdit->text());
            mainwindow->show();
            db.close();

            QSettings settings("settings.ini",QSettings::IniFormat);
            settings.beginGroup("login");
            settings.setValue("lastuser",ui->userLineEdit->text());
            if(ui->checkBox->isChecked())
            {
                settings.setValue("rempass", "1");
                settings.setValue("pass", stringXOR(ui->passLineEdit->text(), ui->userLineEdit->text()));
            }
            else settings.setValue("rempass", "0");

            this->close();
        }
    }
}


void LoginDialog::on_regPushButton_clicked()
{
    QSqlDatabase db;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user_pass.db");
    QSqlQuery sq(db);
    db.open();

    QString look_for_this = "select user, pass, level from users where user = '" + ui->userLineEdit->text() + "'";
    sq.prepare(look_for_this);
    sq.exec();
    sq.first();
    int size = sq.at() + 1;
    if(size <= 0)
    {
        QMessageBox::information(this,"QSL Manager","This User is not Registered yet.",QMessageBox::Ok);
        db.close();
    }
    else
    {
        QString pass = stringXOR(sq.value(1).toString(), ui->userLineEdit->text());
        int level = sq.value(2).toInt();
        if (pass != ui->passLineEdit->text())
        {
            QMessageBox::information(this,"QSL Manager","Wrong Username or Password!",QMessageBox::Ok);
            db.close();
        }
        else if(level)
        {
            QMessageBox::information(this,"QSL Manager","This User is not Permitted to Create New User.",QMessageBox::Ok);
            db.close();
        }
        else
        {
            RegDialog *regdialog = new RegDialog();
            regdialog->show();
            db.close();
            this->close();
        }
    }
}

