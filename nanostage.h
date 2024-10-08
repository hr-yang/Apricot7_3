#ifndef NANOSTAGE_H
#define NANOSTAGE_H

#include <QDialog>
#include <QSettings>


namespace Ui {
class nanostage;
}



class NanoStage
{
public:
    NanoStage(void){};
    virtual ~NanoStage(void){};

                                    //虚函数主要作用是“运行时多态”，通过多态实现的父类调用子类成员函数，这里面必须要有虚函数，并且在子类里面重写。
    virtual bool InitStage(){return 0;}     //不能使用纯虚函数,因为包含纯虚函数的类被称为是“抽象类”，抽象类不能使用new出对象，只有实现了这个纯虚函数的子类才能new出对象。纯虚函数更像是“只提供申明，没有实现”，是对子类的约束，是“接口继承”。
    virtual bool ConnectStage(){return 0;}
    virtual bool DisconnectStage(){return 0;}
    virtual bool SetAxisServo(int){return 0;}
    virtual bool MoveAxis(double value){Q_UNUSED(value);return 0;}
    virtual double ReadAxisPosition(){return 0;}
    virtual bool ReadAxisLimit(int dir){Q_UNUSED(dir);return 0;}
    virtual void Stop(){}
};





class nanostage : public QDialog
{
    Q_OBJECT

public:
    explicit nanostage(QWidget *parent = 0);
    ~nanostage();

    void closeEvent(QCloseEvent *event);


private:
    Ui::nanostage *ui;

    QSettings* settings;

    NanoStage* nanoStageChannel1;
    NanoStage* nanoStageChannel2;
    double channel1StepValue;
    double channel2StepValue;






private slots:

        void connectNanoStageSlot();
        void disconnectNanoStageSlot();
        void setStepValueNanoStageSlot();
        void LeftNanoStageSlot();
        void RightNanoStageSlot();
        void BackNanoStageSlot();
        void FrontNanoStageSlot();
        void setXNanoStageServoSlot(int state);
        void setYNanoStageServoSlot(int state);

signals:
        void sendCloseSgn(const QString &);//向主窗口发送关闭信号，让主窗口消除注册的句柄

};

#endif // NANOSTAGE_H
