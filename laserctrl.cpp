#include "laserctrl.h"
#include "ui_laserctrl.h"
#include <QEventLoop>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include <QThread>

LaserCtrl::LaserCtrl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LaserCtrl),
    axWidget(NULL),
    axWidget_2(NULL)
{
    ui->setupUi(this);

    this->setFixedSize(230,360);

    axWidget = new QAxWidget(this);
    axWidget->setControl(QStringLiteral("{3ce35bf3-1e13-4d2c-8c0b-def6314420b3}"));
    axWidget->setObjectName(QStringLiteral("axWidget"));
    axWidget->setProperty("APTHelp", QVariant(false));
    axWidget->setProperty("DISPLAYMODE", QVariant(1));
    axWidget->setProperty("HWSerialNum", QVariant(85855438));
    axWidget->setProperty("enabled", QVariant(true));

    axWidget->dynamicCall("StartCtrl()");

    axWidget->hide();

    ui->pushButton_ON->setEnabled(false);
    ui->pushButton_OFF->setEnabled(false);

    ui->pushButton_Lightsheet->setEnabled(true);
    ui->pushButton_TPM->setEnabled(false);



    axWidget_2 = new QAxWidget(this);
    axWidget_2->setControl(QStringLiteral("{3ce35bf3-1e13-4d2c-8c0b-def6314420b3}"));
    axWidget_2->setObjectName(QStringLiteral("axWidget"));
    axWidget_2->setProperty("APTHelp", QVariant(false));
    axWidget_2->setProperty("DISPLAYMODE", QVariant(1));
    axWidget_2->setProperty("HWSerialNum", QVariant(85851915));
    axWidget_2->setProperty("enabled", QVariant(true));

    axWidget_2->dynamicCall("StartCtrl()");

    axWidget_2->hide();

    ui->pushButton_ON_2->setEnabled(true);
    ui->pushButton_OFF_2->setEnabled(false);

    qDebug()<<"laser thread id: "<<QThread::currentThreadId();

}

LaserCtrl::~LaserCtrl()
{
     axWidget->dynamicCall("StopCtrl()");
     axWidget_2->dynamicCall("StopCtrl()");
     delete ui;
}


// 重写closeEvent: 确认退出对话框
void LaserCtrl::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton button;
    button=QMessageBox::question(this,tr("exit dialog"),QString(tr("switch to Two Photon Mode?")),QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No);
    if(button==QMessageBox::Cancel)
    {
         event->ignore(); // 忽略退出信号，程序继续进行
    }
    else if(button==QMessageBox::No)
    {

         emit sendCloseSgn("laserctrl");//发送注销子窗口句柄信号
         event->accept(); // 接受退出信号，程序退出
    }
    else if(button==QMessageBox::Yes)
    {
        on_pushButton_TPM_clicked();//切换为TPM模式
        emit sendCloseSgn("laserctrl");//发送注销子窗口句柄信号
        event->accept(); // 接受退出信号，程序退出
    }
}



void LaserCtrl::on_pushButton_Lightsheet_clicked()
{

    QEventLoop loop;
    QTimer::singleShot(200, &loop, SLOT(quit()));
    loop.exec();
    axWidget->dynamicCall("SC_SetOperatingMode(0,1)");


    ui->pushButton_ON->setEnabled(true);
    ui->pushButton_OFF->setEnabled(false);


    ui->pushButton_Lightsheet->setEnabled(false);
    ui->pushButton_TPM->setEnabled(true);

}

void LaserCtrl::on_pushButton_TPM_clicked()
{
    QEventLoop loop;
    QTimer::singleShot(200, &loop, SLOT(quit()));
    loop.exec();

    axWidget->dynamicCall("SC_SetOperatingMode(0,4)");

    QTimer::singleShot(200, &loop, SLOT(quit()));
    loop.exec();
    axWidget->dynamicCall("SC_Enable(0)");

    ui->pushButton_ON->setEnabled(false);
    ui->pushButton_OFF->setEnabled(false);

    ui->pushButton_Lightsheet->setEnabled(true);
    ui->pushButton_TPM->setEnabled(false);
}

void LaserCtrl::on_pushButton_OFF_clicked()
{
     axWidget->dynamicCall("SC_Disable(0)");
     ui->pushButton_ON->setEnabled(true);
     ui->pushButton_OFF->setEnabled(false);
}

void LaserCtrl::on_pushButton_ON_clicked()
{
     axWidget->dynamicCall("SC_Enable(0)");
     ui->pushButton_ON->setEnabled(false);
     ui->pushButton_OFF->setEnabled(true);
}

void LaserCtrl::on_pushButton_ON_2_clicked()
{
    axWidget_2->dynamicCall("SC_Enable(0)");
    ui->pushButton_ON_2->setEnabled(false);
    ui->pushButton_OFF_2->setEnabled(true);
}

void LaserCtrl::on_pushButton_OFF_2_clicked()
{
    axWidget_2->dynamicCall("SC_Disable(0)");
    ui->pushButton_ON_2->setEnabled(true);
    ui->pushButton_OFF_2->setEnabled(false);
}
