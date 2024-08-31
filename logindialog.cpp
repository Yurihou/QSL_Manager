#include "logindialog.h"
#include "./ui_logindialog.h"

#include "mainwindow.h"
#include "regdialog.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>

QString stringXOR(QString str, QString key)
{
    if(key.isEmpty())key = "114514";
    while(key.length() < str.length())key.append(key);
    if(key.length() > str.length())key = key.left(str.length());
    QByteArray bs1 = str.toLatin1();
    QByteArray bs2 = key.toLatin1();
    for(int i=0; i<str.length(); i++)
    {
        bs1[i] = bs1[i] ^ bs2[i];
    }

    QString result;
    return result.prepend(bs1);
}

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


        create_sql = "insert into users(user, pass, phone, level) values('root', '8888', '114514', 0)";
        sq.prepare(create_sql);
        sq.exec();
    }
    else if(sq.size()==1)
    {
        qDebug() << "Error: sql database is right.";
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
        qDebug() << "This User is not Registered yet.";
        db.close();
    }
    else
    {
        QString pass = sq.value(1).toString();
        int level = sq.value(2).toInt();
        if (pass != ui->passLineEdit->text())
        {
            qDebug() << "Wrong Username or Password!";
            db.close();
        }
        else if(level == 0)
        {
            qDebug() << "This is root user. Root user function has not developed yet.";
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
        qDebug() << "This User is not Registered yet.";
        db.close();
    }
    else
    {
        QString pass = sq.value(1).toString();
        int level = sq.value(2).toInt();
        if (pass != ui->passLineEdit->text())
        {
            qDebug() << "Wrong Username or Password!";
            db.close();
        }
        else if(level)
        {
            qDebug() << "This User is not Permitted to Create new user.";
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

