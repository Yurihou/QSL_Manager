#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void receiveUserData(QString data);
    void receiveRefresh();

    void on_addQSOPushButton_clicked();

    void on_toolButton_clicked();

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

signals:
    void sendData1(QString);
    void sendData2(QString, QString);
    void sendData3(QString, QStringList);

private:
    Ui::MainWindow *ui;

    QString currentCall;

    void refreshTable();
};

#endif // MAINWINDOW_H
