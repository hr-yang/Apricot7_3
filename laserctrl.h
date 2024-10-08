#ifndef LASERCTRL_H
#define LASERCTRL_H

#include <QDialog>
#include <QAxWidget>
#include <QCloseEvent>


namespace Ui {
class LaserCtrl;
}

class LaserCtrl : public QDialog
{
    Q_OBJECT

public:
    explicit LaserCtrl(QWidget *parent = 0);
    ~LaserCtrl();

private:
    Ui::LaserCtrl *ui;
    QAxWidget *axWidget;
    QAxWidget *axWidget_2;


    void closeEvent(QCloseEvent *event);    // 重写closeEvent的申明

private slots:

    void on_pushButton_Lightsheet_clicked();
    void on_pushButton_TPM_clicked();
    void on_pushButton_OFF_clicked();
    void on_pushButton_ON_clicked();    

    void on_pushButton_ON_2_clicked();

    void on_pushButton_OFF_2_clicked();

signals:
        void sendCloseSgn(const QString &);//向主窗口发送关闭信号，让主窗口消除注册的句柄
};

#endif // LASERCTRL_H
