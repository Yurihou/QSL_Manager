#ifndef ADDQSODIALOG_H
#define ADDQSODIALOG_H

#include <QDialog>

namespace Ui {
class AddQSODialog;
}

class AddQSODialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddQSODialog(QWidget *parent = nullptr);
    ~AddQSODialog();

private slots:
    void receiveUserDataFromAddQso(QString myCallsign, QString callsign);
    void reveiveQsoDataFromMain(QString myCallsign, QStringList list);

    void on_calcelPushButton_clicked();

    void on_addQSOPushButton_clicked();
    void on_modeComboBox_currentTextChanged(const QString &arg1);

    void on_deletePushButton_clicked();

signals:
    void sendRefresh();
private:
    Ui::AddQSODialog *ui;

    QString currentCall;
    int currentQSOId;
};

#endif // ADDQSODIALOG_H
