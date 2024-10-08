#ifndef KOKIDIALOG_H
#define KOKIDIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QSettings>



namespace Ui {
class KokiDialog;
}

class KokiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KokiDialog(QWidget *parent = 0,QString stage1="",QString label1="",float resolu1=0,int precision1=0,
                                            QString stage2="",QString label2="",float resolu2=0,int precision2=0);
    ~KokiDialog();

    void closeEvent(QCloseEvent *event);

private slots:
    void readSerialDataSlot();

    void on_pushButton_SetSpeed_clicked();

    void on_pushButton_Home_clicked();

    void on_pushButton_OpenCom_clicked();

    void on_radioButton_Free_clicked();

    void on_radioButton_Hold_clicked();

    void on_pushButton_Stop_clicked();

    void on_pushButton_Plus_clicked();

    void on_pushButton_Minus_clicked();

    void on_pushButton_SetA_clicked();

    void on_pushButton_SetB_clicked();

    void on_pushButton_SetC_clicked();

    void on_pushButton_A_clicked();

    void on_pushButton_B_clicked();

    void on_pushButton_C_clicked();

    void on_pushButton_SetSpeed_2_clicked();

    void on_pushButton_Home_2_clicked();

    void on_radioButton_Free_2_clicked();

    void on_radioButton_Hold_2_clicked();

    void on_pushButton_Stop_2_clicked();

    void on_pushButton_Plus_2_clicked();

    void on_pushButton_Minus_2_clicked();

    void on_pushButton_SetA_2_clicked();

    void on_pushButton_SetB_2_clicked();

    void on_pushButton_SetC_2_clicked();

    void on_pushButton_A_2_clicked();

    void on_pushButton_B_2_clicked();

    void on_pushButton_C_2_clicked();

private:
    Ui::KokiDialog *ui;

    QSerialPort *serial;

    QSettings* settings;

    char  isClose;

    QByteArray buffer;
    uchar flag;
    uchar finished_flag;
    uchar ready;

    QString dialogName;

    QString stage_dir1;
    QString stage_dir2;

    int precis1;//精度，界面显示小数点后位数
    int precis2;

    float resolution; // um/pluse
    float resolution_2;


    int A;// pulse
    int B;
    int C;

    int A_2;// pulse
    int B_2;
    int C_2;

    int   range;
    int   slow;// pulse/s
    int   fast;
    int   rate;// ms
    int   slow_2;
    int   fast_2;
    int   rate_2;

signals:
    void sendCloseSgn(const QString &);//向主窗口发送关闭信号，让主窗口消除注册的句柄
};

#endif // KOKIDIALOG_H
