#ifndef REGDIALOG_H
#define REGDIALOG_H

#include <QDialog>

namespace Ui {
class RegDialog;
}

class RegDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegDialog(QWidget *parent = nullptr);
    ~RegDialog();

private slots:
    void on_pushButton_clicked();

    void on_OKPushButton_clicked();

private:
    Ui::RegDialog *ui;
};

#endif // REGDIALOG_H
