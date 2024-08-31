#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_loginPushButton_clicked();

    void on_regPushButton_clicked();

signals:
    void sendData(QString);

private:
    Ui::LoginDialog *ui;
};
#endif // LOGINDIALOG_H
