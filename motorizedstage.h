#ifndef MOTORIZEDSTAGE_H
#define MOTORIZEDSTAGE_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTcpSocket>
#include <qtimer.h>
#include <QQueue>
#include <QSettings>

#include "mytcpsocket.h"


//motorizedstage虚类
namespace Ui {
class motorizedstage;
}

class MotorizedStage
{
public:
    MotorizedStage(void){}
    virtual ~MotorizedStage(void){}

    double speed_value_;

    //虚函数主要作用是“运行时多态”，通过多态实现的父类调用子类成员函数，这里面必须要有虚函数，并且在子类里面重写。
    //不能使用纯虚函数,因为包含纯虚函数的类被称为是“抽象类”，抽象类不能使用new出对象，只有实现了这个纯虚函数的子类才能new出对象。纯虚函数更像是“只提供申明，没有实现”，是对子类的约束，是“接口继承”。
    virtual bool InitStage(){return 0;}//虚函数的默认实现
    virtual bool ConnectStage(){return 0;}
    virtual bool DisconnectStage(){return 0;}
    virtual void SetParamters(){}
    virtual void set_speed_value(double speed_value){Q_UNUSED(speed_value);}
    virtual bool MoveAxis(double value){Q_UNUSED(value);return 0;}
    virtual double ReadAxisPosition(){return 0;}
    virtual bool ReadAxisLimit(int dir){Q_UNUSED(dir);return 0;}
    virtual void Stop(){}
};



class motorizedstage : public QDialog
{
    Q_OBJECT

public:
    explicit motorizedstage(QWidget *parent = 0);
    ~motorizedstage();

    void closeEvent(QCloseEvent *event);

    double x_convert;
    double y_convert;
    double z_convert;
    double u_convert;

    QSettings* settings;

    MyTcpSocket*  tcp;
    QThread*      tcpThread;

    QTimer*   timer;

     //用于台子自动走相关变量
     double startPosition;
     int    stepNumber;
     double distancePerStep;
     int    exposureTime;
     int    stepCnt;   //计数变量
     double aimPos;
     int    status;
     int    autoRunMode;

     double zPositionOffset;

private:
    Ui::motorizedstage *ui;


private slots:
        void ConnnctMotorizedStageButtonSlot();

        void LeftMotorizedStageButtonSlot();
        void RightMotorizedStageButtonSlot();
        void UpMotorizedStageButtonSlot();
        void DownMotorizedStageButtonSlot();
        void FrontMotorizedStageButtonSlot();
        void BackMotorizedStageButtonSlot();
        void ClockwiseMotorizedStageButtonSlot();
        void AntiClockMotorizedStageButtonSlot();

        void unlockZbuttonSlot();
        void lockZbuttonSlot();//定时锁住Z轴按钮
        void setZeroSlot();

        void setSpeedButtonSlot();
        void stopButtonSlot();

        void autoRunButtonSlot();
        void clearTextSlot();

        void isExposureSlot();//判断是否可以进行曝光

        void updataStateSlot();

        void messageBoxSlot(const QString &);


signals:
        void startExposureSgn();//向主窗口发送曝光信号，通过主窗口调用另一个子窗口的槽函数
        void sendCloseSgn(const QString &);//向主窗口发送关闭信号，让主窗口消除注册的句柄

        void sendConnect();
        void sendDisconnect();
};

#endif // MOTORIZEDSTAGE_H
