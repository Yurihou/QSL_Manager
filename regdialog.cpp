#include "regdialog.h"
#include "ui_regdialog.h"

#include "logindialog.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

RegDialog::RegDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegDialog)
{
    ui->setupUi(this);
    ui->userLineEdit->setFocus();
}

RegDialog::~RegDialog()
{
    delete ui;
}

void RegDialog::on_pushButton_clicked()
{
    LoginDialog *logindialog = new LoginDialog();
    logindialog->show();
    this->close();
}


void RegDialog::on_OKPushButton_clicked()
{
    QSqlDatabase db;

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user_pass.db");
    QSqlQuery sq(db);
    db.open();

    QString look_for_this = "select user from users where user = '" + ui->userLineEdit->text() + "'";
    sq.prepare(look_for_this);
    sq.exec();
    sq.first();
    int size = sq.at() + 1;
    if(size > 0)
    {
        qDebug() << "This Username is already in Use.";
        db.close();
    }
    else if(ui->passLineEdit->text() != ui->passCfmLineEdit->text())
    {
        qDebug() << "Two passwords are not Identical. Please Check your Password.";
        db.close();
    }
    else
    {
        QString look_for_this = "insert into users (user, pass, phone, level) values('"
                                + ui->userLineEdit->text() + "', '"
                                + ui->passLineEdit->text() + "', '"
                                + ui->phoneLineEdit->text() + "', 1)";
        sq.prepare(look_for_this);
        sq.exec();

        //create this user's qsl database
        bool aaa = false;
        QSqlDatabase db2;

        db2 = QSqlDatabase::addDatabase("QSQLITE");
        db2.setDatabaseName(QString("log_%1.db").arg(ui->userLineEdit->text()));
        aaa = db2.open();
        if (!aaa)qDebug() << "Error: Failed to connect database." << db2.lastError();
        QSqlQuery sq2(db2);

        QString create_sql = "create table log ("
                             "id integer primary key autoincrement, "
                             "date integer not null, "
                             "utc integer not null, "
                             "call text not null, "
                             "band text not null, "
                             "freq real, "
                             "mode text no null, "
                             "rst_r text, "
                             "rst_s text, "
                             "summary text, "
                             "qsl_r integer, "
                             "qsl_s integer, "
                             "note text)";
        sq2.prepare(create_sql);
        sq2.exec();

        db2.close();

        LoginDialog *logindialog = new LoginDialog();
        logindialog->show();
        db.close();
        this->close();
    }
}

