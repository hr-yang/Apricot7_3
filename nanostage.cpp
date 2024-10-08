#include "nanostage.h"
#include "ui_nanostage.h"
#include "SC200.h"
#include "qt_settings_widgetdatacontrol.h"

#include <QSettings>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>
#include <QCloseEvent>
#include <QStandardPaths>


nanostage::nanostage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::nanostage),
    settings(NULL)
{
    ui->setupUi(this);

    this->setFixedSize(650,650);

    nanoStageChannel1=new SC200Channel1;
    nanoStageChannel2=new SC200Channel2;



    connect(ui->connectNanoStageButton,&QPushButton::clicked,this,&nanostage::connectNanoStageSlot);
    connect(ui->disconnectNanoStageButton,&QPushButton::clicked,this,&nanostage::disconnectNanoStageSlot);
    connect(ui->setNanoStepButton,&QPushButton::clicked,this,&nanostage::setStepValueNanoStageSlot);
    connect(ui->leftNanoStageButton,&QPushButton::clicked,this,&nanostage::LeftNanoStageSlot);
    connect(ui->rightNanoStageButton,&QPushButton::clicked,this,&nanostage::RightNanoStageSlot);
    connect(ui->backNanoStageButton,&QPushButton::clicked,this,&nanostage::BackNanoStageSlot);
    connect(ui->frontNanoStageButton,&QPushButton::clicked,this,&nanostage::FrontNanoStageSlot);
    connect(ui->xServoCheckBox,&QCheckBox::stateChanged,this,&nanostage::setXNanoStageServoSlot);
    connect(ui->yServoCheckBox,&QCheckBox::stateChanged,this,&nanostage::setYNanoStageServoSlot);



    //创建和读取一样  必须在save之后磁盘才会有该文件
    settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/ApricotConfig.ini", QSettings::IniFormat,this);
    settings->setIniCodec("UTF-8");

    Qt_Settings_WidgetDataControl::read(settings,this, Qt_Settings_WidgetDataControl::controlFlgasWidgets);

    setAttribute(Qt::WA_DeleteOnClose);//因为当关闭子窗体时不会调用析构函数，只有关闭父窗体时，才会调用子窗体的析构函数，解决办法加入这句语句


}

nanostage::~nanostage()
{

     Qt_Settings_WidgetDataControl::save(settings,this, Qt_Settings_WidgetDataControl::controlFlgasWidgets);


    delete ui;


}

void nanostage::closeEvent(QCloseEvent *event)
{
    emit sendCloseSgn("nanostage");//发送注销子窗口句柄信号
    event->accept();
}


void nanostage::connectNanoStageSlot()
{
    if(nanoStageChannel1->ConnectStage())
    {
        QMessageBox::information(NULL, "nano stage", "connect success!", QMessageBox::Yes, QMessageBox::Yes);

        channel1StepValue=0;
        channel2StepValue=0;


    }
    else
    {
        QMessageBox::information(NULL, "nano stage", "connect failed!", QMessageBox::Yes, QMessageBox::Yes);
    }

}

void nanostage::disconnectNanoStageSlot()
{
    nanoStageChannel1->DisconnectStage();
}

void nanostage::setStepValueNanoStageSlot()
{
    channel1StepValue=ui->xNanoStepLineEdit->text().toDouble();
    channel2StepValue=ui->yNanoStepLineEdit->text().toDouble();
}

void nanostage::LeftNanoStageSlot()
{
    nanoStageChannel1->MoveAxis(channel1StepValue);
}

void nanostage::RightNanoStageSlot()
{
    nanoStageChannel1->MoveAxis(-channel1StepValue);
}

void nanostage::BackNanoStageSlot()
{
    nanoStageChannel2->MoveAxis(channel2StepValue);
}

void nanostage::FrontNanoStageSlot()
{
    nanoStageChannel2->MoveAxis(-channel2StepValue);
}

void nanostage::setXNanoStageServoSlot(int state)
{
    if(state==Qt::Checked)
        nanoStageChannel1->SetAxisServo(0);
    else
        nanoStageChannel1->SetAxisServo(1);
}

void nanostage::setYNanoStageServoSlot(int state)
{
    if(state==Qt::Checked)
        nanoStageChannel2->SetAxisServo(0);
    else
        nanoStageChannel2->SetAxisServo(1);
}
