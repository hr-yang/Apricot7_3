#include "motorizedstage.h"
#include "ui_motorizedstage.h"
#include "qt_settings_widgetdatacontrol.h"


#include <QSettings>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QThread>
#include <QListWidget>
#include <QStackedWidget>
#include <QCheckBox>
#include <QKeyEvent>
#include <QLCDNumber>
#include <QComboBox>
#include <QStandardPaths>

motorizedstage::motorizedstage(QWidget *parent) :
    QDialog(parent),
    settings(NULL),
    tcp(NULL),
    tcpThread(NULL),
    timer(NULL),
    ui(new Ui::motorizedstage)//指针一定要进行初始化！！！
{
    ui->setupUi(this);

    this->setFixedSize(680,760);

    //连接信号槽
        connect(ui->connect_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::ConnnctMotorizedStageButtonSlot);

        connect(ui->left_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::LeftMotorizedStageButtonSlot);
        connect(ui->right_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::RightMotorizedStageButtonSlot);
        connect(ui->back_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::BackMotorizedStageButtonSlot);
        connect(ui->front_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::FrontMotorizedStageButtonSlot);
        connect(ui->up_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::UpMotorizedStageButtonSlot);
        connect(ui->down_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::DownMotorizedStageButtonSlot);
        connect(ui->clockwise_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::ClockwiseMotorizedStageButtonSlot);
        connect(ui->anticlock_motorized_stage_button, &QPushButton::clicked, this, &motorizedstage::AntiClockMotorizedStageButtonSlot);

        connect(ui->pushButton_unlock,&QPushButton::clicked,this,&motorizedstage::unlockZbuttonSlot);
        connect(ui->pushButton_SetZero,&QPushButton::clicked,this,&motorizedstage::setZeroSlot);

        connect(ui->setSpeedButton,&QPushButton::clicked,this,&motorizedstage::setSpeedButtonSlot);
        connect(ui->stopButton,&QPushButton::clicked,this,&motorizedstage::stopButtonSlot);

        connect(ui->pushButton_ClearText,&QPushButton::clicked,this,&motorizedstage::clearTextSlot);
        connect(ui->autoRunButton,&QPushButton::clicked,this,&motorizedstage::autoRunButtonSlot);

        connect(ui->comboBoxAxisSpeed,SIGNAL(currentIndexChanged(int)),ui->stackedWidget,SLOT(setCurrentIndex(int)));


        zPositionOffset=0;


        //关闭up down 两个按钮，防止意外操作触碰到物镜
        ui->up_motorized_stage_button->setEnabled(false);
        ui->down_motorized_stage_button->setEnabled(false);


        timer =new QTimer;
        timer->setInterval(100);
        connect(timer,&QTimer::timeout,this,&motorizedstage::updataStateSlot);
        timer->start(100);


        //使用一个线程跑tcp
        tcpThread = new QThread;
        tcp       = new MyTcpSocket;
        connect(this,&motorizedstage::sendConnect,tcp,&MyTcpSocket::connect);
        connect(this,&motorizedstage::sendDisconnect,tcp,&MyTcpSocket::disconnect);
        connect(tcp,&MyTcpSocket::sendMessageBox,this,&motorizedstage::messageBoxSlot);

        //QThread 应该被看做是操作系统线程的接口或控制点，而不应该包含需要在新线程中运行的代码。需要运行的代码应该放到一个QObject的子类中，然后将该子类的对象moveToThread到新线程中。
        //QThread是用来管理子线程的，也就是run启动的线程。
        //QThread 所依附的线程，就是执行new QThread的线程(是主线程)
        //obj->moveToThread(thread)的含义：把obj对象依附到子线程中(通过movetoThread)，obj的槽函数在子线程运行（槽函数在接收者(ojb)所依附的线程执行）
        tcp->moveToThread(tcpThread);//槽函数在信号接收者(tcp)所依附的线程(tcpThread::run)执行
       //从哪个线程中调用发射信号语句emit sgn，信号就在哪个线程中

        tcpThread->start();



        //创建和读取一样  必须在save之后磁盘才会有该文件
        settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/ApricotConfig.ini", QSettings::IniFormat,this);
        settings->setIniCodec("UTF-8");

        //读取上一次配置
        Qt_Settings_WidgetDataControl::read(settings,this, Qt_Settings_WidgetDataControl::controlFlgasWidgets);


        //多页面部件设置
        ui->stackedWidget->setCurrentIndex(ui->comboBoxAxisSpeed->currentIndex());



        //修改按键文字
        ui->connect_motorized_stage_button->setText("Connect");



        setAttribute(Qt::WA_DeleteOnClose);//因为当关闭子窗体时不会调用析构函数，只有关闭父窗体时，才会调用子窗体的析构函数，解决办法加入这句语句

        qDebug()<<"motorized thread id: "<<QThread::currentThreadId();
}

motorizedstage::~motorizedstage()
{

    tcp->isConnect=0;

    tcpThread->quit();
    tcpThread->wait();

    Qt_Settings_WidgetDataControl::save(settings,this, Qt_Settings_WidgetDataControl::controlFlgasWidgets);

    delete timer;
    delete tcp;
    delete tcpThread;

    delete ui;
}

//窗口关闭事件，保证一次只能打开一个窗口
void motorizedstage::closeEvent(QCloseEvent *event)
{
    emit sendCloseSgn("motorizedstage");//发送注销子窗口句柄信号
    event->accept();
}




//连接台子
void motorizedstage::ConnnctMotorizedStageButtonSlot()
{
    if(ui->connect_motorized_stage_button->text() == tr("Connect"))
    {

        //向tcp发送连接请求
        emit sendConnect();

        //修改按键文字
        ui->connect_motorized_stage_button->setText("Disconnect");

        //U轴
        tcp->tcpQueue.enqueue("COMMAND:SPEEDRATIO X250;");/*设置旋转台转速比(n*百分之一度/1圈)*/
        tcp->tcpQueue.enqueue("COMMAND:MICROSTEPS X200;");//设置指定轴的细分数 范围 1-512 ，可用 PMC400_Get_MicroSteps 查询
        tcp->tcpQueue.enqueue("COMMAND:RESOLUTIONRATIO X0;");//光栅尺分辨率（注：此处为脉冲/毫米，即1MM对应多少脉冲）,开环请设为0
        u_convert=20*200/2.5;//此电机20个脉冲一圈   1600

        //x轴
        tcp->tcpQueue.enqueue("COMMAND:SCREWLEAD Y2000000;");//丝杆导程1圈-2mm
        tcp->tcpQueue.enqueue("COMMAND:MICROSTEPS Y64;");//设置指定轴的细分数 //64细分
        tcp->tcpQueue.enqueue("COMMAND:RESOLUTIONRATIO Y10000;");//光栅尺分辨率（注：此处为脉冲/毫米，即1MM对应多少脉冲）,开环请设为0
        x_convert=200*64/2; // 丝杆导程1圈-2mm     1mm-0.5圈               1mm  - 0.5* (200 *8)cnt

        //Y轴
        tcp->tcpQueue.enqueue("COMMAND:SCREWLEAD Z2000000;");//丝杆导程（纳米）  1圈-2mm
        tcp->tcpQueue.enqueue("COMMAND:MICROSTEPS Z8;");//设置指定轴的细分数
        tcp->tcpQueue.enqueue("COMMAND:RESOLUTIONRATIO Z0;");//光栅尺分辨率（注：此处为脉冲/毫米，即1MM对应多少脉冲）,开环请设为0
        y_convert = 200 * 8 / 2;

        //z轴
        tcp->tcpQueue.enqueue("COMMAND:SCREWLEAD U1578500;");//丝杆导程（纳米）  1圈-2mm
        tcp->tcpQueue.enqueue("COMMAND:MICROSTEPS U200;");//设置指定轴的细分数
        tcp->tcpQueue.enqueue("COMMAND:RESOLUTIONRATIO U10000;");//光栅尺分辨率（注：此处为脉冲/毫米，即1MM对应多少脉冲）,开环请设为0
        z_convert = 200 * 200 / 1.5785f;


        //设置台子速度
        setSpeedButtonSlot();


        tcp->tcpQueue.enqueue("COMMAND:SAVE;");//注意保存设置

        //显示控制器SN号
        ui->labelPMC400SN->setText("SN:"+QString::number(tcp->pmc400Status.PMC400_SN));

    }
    else
    {
        //向tcp发送断开连接请求
        emit sendDisconnect();

        //修改按键文字
        ui->connect_motorized_stage_button->setText("Connect");
    }
}


void motorizedstage::messageBoxSlot(const QString & str)
{

    QMessageBox::information(NULL, "motorized stage", str, QMessageBox::Yes, QMessageBox::Yes);

    if(str=="disconnected!"||str=="connect failed!")//台子掉线或未连接成功
        ui->connect_motorized_stage_button->setText("Connect");//修改按键文字
}


void motorizedstage::setZeroSlot()
{
    zPositionOffset=tcp->pmc400Status.PMC400_U_PositionEncode/10000.0f;
}



void motorizedstage::updataStateSlot()
{

        ui->xPosLineEdit->setText(QString::number(tcp->pmc400Status.PMC400_Y_PositionEncode/10000.0f,10,4));
        ui->zPosLineEdit->setText(QString::number((tcp->pmc400Status.PMC400_U_PositionEncode/10000.0f-zPositionOffset),10,4));

        if(tcp->pmc400Status.PMC400_XYZU_EL_ORG&0x2000)   ui->left_motorized_stage_button->setEnabled(false);
        else        ui->left_motorized_stage_button->setEnabled(true);

        if(tcp->pmc400Status.PMC400_XYZU_EL_ORG&0x1000)  ui->right_motorized_stage_button->setEnabled(false);
        else 		ui->right_motorized_stage_button->setEnabled(true);


        if(tcp->pmc400Status.PMC400_XYZU_EL_ORG&0x200000)   ui->front_motorized_stage_button->setEnabled(false);
        else        ui->front_motorized_stage_button->setEnabled(true);

        if(tcp->pmc400Status.PMC400_XYZU_EL_ORG&0x100000)  ui->back_motorized_stage_button->setEnabled(false);
        else 		ui->back_motorized_stage_button->setEnabled(true);


//      if(pmc400Status.PMC400_XYZU_EL_ORG&0x20000000)   ui->down_motorized_stage_button->setEnabled(false);
//      else        ui->down_motorized_stage_button->setEnabled(true);

//      if(pmc400Status.PMC400_XYZU_EL_ORG&0x10000000)  ui->up_motorized_stage_button->setEnabled(false);
//      else 		ui->up_motorized_stage_button->setEnabled(true);

//      ui->autoTextBrowser->append(QString::number(pmc400Status.PMC400_Y_Is_Moveing));
//      ui->autoTextBrowser->moveCursor(QTextCursor::End);

        if(autoRunMode==1)
        {
            if(status==0)
            {

                if(stepCnt!=0)//发送位置移动指令   第0个位置不走台子
                {
                    int pulse=distancePerStep*x_convert/1000.0f;//一般是走3um   um转换为mm
                    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE Y"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
                    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动
                }
                QTimer::singleShot(1500, this, SLOT(isExposureSlot()));//1.5s后判断台子是否停止(经测试1.5s比较合理)

                status=1; //下次必须进入下一个状态(判断台子是否停止状态)
            }
        }
}


void motorizedstage::isExposureSlot()
{
    if(tcp->pmc400Status.PMC400_Y_Is_Moveing==1)//,才能走下一步
    {
        emit startExposureSgn();//向主窗口发送曝光信号如果台子停住了
        aimPos=distancePerStep*stepCnt+startPosition;
        ui->autoTextBrowser->append(QString("Step:%1,AimPos:%2 um,curPos:%3 um").arg(QString::number(stepCnt)).arg(QString::number(aimPos,10,1)).arg(QString::number(tcp->pmc400Status.PMC400_Y_PositionEncode/10.0f,10,1)));
        ui->autoTextBrowser->moveCursor(QTextCursor::End); //打印当前位置

        QEventLoop loop;
        QTimer::singleShot(exposureTime+100, &loop, SLOT(quit()));//非阻塞延时一个曝光时间
        loop.exec();

        stepCnt++;
        if(stepCnt==stepNumber)
        {
            autoRunMode=0;//走完,关闭此模式
            ui->autoTextBrowser->append("Auto motion stop!");          

            //恢复为原速度
            int tempvalue=ui->lineEditStartSpd_X->text().toDouble()*x_convert;
            tcp->tcpQueue.enqueue(("COMMAND:STARTSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//初速度（mm/s）
            tempvalue=ui->lineEditAccSpd_X->text().toDouble()*x_convert;
            tcp->tcpQueue.enqueue(("COMMAND:ACCSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//加速度（mm/s2）
            tempvalue=ui->lineEditMaxSpd_X->text().toDouble()*x_convert;
            tcp->tcpQueue.enqueue(("COMMAND:FINALSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//加速阶段终(匀速)速度（mm/s）
        }
        status=0;  //如果台子停住了,才能进发指令状态
    }
    else//台子没有停住 150ms后再判断一次
    {
        qDebug()<<"still running";
        QTimer::singleShot(150, this, SLOT(isExposureSlot()));
    }
}


//台子方向移动按钮
void motorizedstage::LeftMotorizedStageButtonSlot()
{
    int pulse=-ui->xStepLineEdit->text().toDouble()*x_convert;
    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE Y"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动
}

//台子方向移动按钮
void motorizedstage::RightMotorizedStageButtonSlot()
{
    int pulse=ui->xStepLineEdit->text().toDouble()*x_convert;
    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE Y"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动

}

//台子方向移动按钮
void motorizedstage::FrontMotorizedStageButtonSlot()
{
    int pulse=-ui->yStepLineEdit->text().toDouble()*y_convert;
    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE Z"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动
}
//台子方向移动按钮
void motorizedstage::BackMotorizedStageButtonSlot()
{
    int pulse=ui->yStepLineEdit->text().toDouble()*y_convert;
    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE Z"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动
}

//台子方向移动按钮
void motorizedstage::UpMotorizedStageButtonSlot()
{
    int pulse=ui->zStepLineEdit->text().toDouble()*z_convert;
    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE U"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动
    qDebug()<<"up button pressed!";
}

//台子方向移动按钮
void motorizedstage::DownMotorizedStageButtonSlot()
{
    int pulse=-ui->zStepLineEdit->text().toDouble()*z_convert;
    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE U"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动
    qDebug()<<"down button pressed!";
}

//台子方向移动按钮
void motorizedstage::ClockwiseMotorizedStageButtonSlot()
{
    int pulse=-ui->uStepLineEdit->text().toDouble()*y_convert;
    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE X"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动
}

//台子方向移动按钮
void motorizedstage::AntiClockMotorizedStageButtonSlot()
{
    int pulse=ui->uStepLineEdit->text().toDouble()*y_convert;
    tcp->tcpQueue.enqueue(("COMMAND:OPENMOVE X"+QString::number(pulse)+";").toLatin1().data());//移动距离（脉冲），开环
    tcp->tcpQueue.enqueue("COMMAND:START;");//在以上设置完毕之后,调用此函数开始移动
}



//速度设置
void motorizedstage::setSpeedButtonSlot()//call this slot to read all steps of user input
{
        int tempvalue=ui->lineEditStartSpd_U->text().toDouble()*u_convert;//lineEdit.toDouble非数字会自动为0
        tcp->tcpQueue.enqueue(("COMMAND:STARTSPEED X"+QString::number(tempvalue)+";").toLatin1().data());//初速度（mm/s）
        tempvalue=ui->lineEditAccSpd_U->text().toDouble()*u_convert;
        tcp->tcpQueue.enqueue(("COMMAND:ACCSPEED X"+QString::number(tempvalue)+";").toLatin1().data());//加速度（mm/s2）
        tempvalue=ui->lineEditMaxSpd_U->text().toDouble()*u_convert;
        tcp->tcpQueue.enqueue(("COMMAND:FINALSPEED X"+QString::number(tempvalue)+";").toLatin1().data());//加速阶段终(匀速)速度（mm/s）

        tempvalue=ui->lineEditStartSpd_X->text().toDouble()*x_convert;
        tcp->tcpQueue.enqueue(("COMMAND:STARTSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//初速度（mm/s）
        tempvalue=ui->lineEditAccSpd_X->text().toDouble()*x_convert;
        tcp->tcpQueue.enqueue(("COMMAND:ACCSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//加速度（mm/s2）
        tempvalue=ui->lineEditMaxSpd_X->text().toDouble()*x_convert;
        tcp->tcpQueue.enqueue(("COMMAND:FINALSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//加速阶段终(匀速)速度（mm/s）


        tempvalue=ui->lineEditStartSpd_Y->text().toDouble()*y_convert;
        tcp->tcpQueue.enqueue(("COMMAND:STARTSPEED Z"+QString::number(tempvalue)+";").toLatin1().data());//初速度（mm/s）
        tempvalue=ui->lineEditAccSpd_Y->text().toDouble()*y_convert;
        tcp->tcpQueue.enqueue(("COMMAND:ACCSPEED Z"+QString::number(tempvalue)+";").toLatin1().data());//加速度（mm/s2）
        tempvalue=ui->lineEditMaxSpd_Y->text().toDouble()*y_convert;
        tcp->tcpQueue.enqueue(("COMMAND:FINALSPEED Z"+QString::number(tempvalue)+";").toLatin1().data());//加速阶段终(匀速)速度（mm/s）


        tempvalue=ui->lineEditStartSpd_Z->text().toDouble()*z_convert;
        tcp->tcpQueue.enqueue(("COMMAND:STARTSPEED U"+QString::number(tempvalue)+";").toLatin1().data());//初速度（mm/s）
        tempvalue=ui->lineEditAccSpd_Z->text().toDouble()*z_convert;
        tcp->tcpQueue.enqueue(("COMMAND:ACCSPEED U"+QString::number(tempvalue)+";").toLatin1().data());//加速度（mm/s2）
        tempvalue=ui->lineEditMaxSpd_Z->text().toDouble()*z_convert;
        tcp->tcpQueue.enqueue(("COMMAND:FINALSPEED U"+QString::number(tempvalue)+";").toLatin1().data());//加速阶段终(匀速)速度（mm/s）

    tcp->tcpQueue.enqueue("COMMAND:SAVE;");//注意保存设置
}


void motorizedstage::clearTextSlot()
{
    ui->autoTextBrowser->clear();
}

void motorizedstage::unlockZbuttonSlot()
{
    ui->up_motorized_stage_button->setEnabled(true);
    ui->down_motorized_stage_button->setEnabled(true);

    QTimer::singleShot(3000, this, SLOT(lockZbuttonSlot()));// 定时锁住z轴按钮，防止意外操作  定时器只执行一次
}

void motorizedstage::lockZbuttonSlot()
{
    ui->up_motorized_stage_button->setEnabled(false);
    ui->down_motorized_stage_button->setEnabled(false);
}



//停止按钮
void motorizedstage::stopButtonSlot()
{
    tcp->tcpQueue.enqueue("COMMAND:STOP;");//即刻停止
    if(autoRunMode==1)
    {
         autoRunMode=0;
        //恢复为原速度
        int tempvalue=ui->lineEditStartSpd_X->text().toDouble()*x_convert;
        tcp->tcpQueue.enqueue(("COMMAND:STARTSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//初速度（mm/s）
        tempvalue=ui->lineEditAccSpd_X->text().toDouble()*x_convert;
        tcp->tcpQueue.enqueue(("COMMAND:ACCSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//加速度（mm/s2）
        tempvalue=ui->lineEditMaxSpd_X->text().toDouble()*x_convert;
        tcp->tcpQueue.enqueue(("COMMAND:FINALSPEED Y"+QString::number(tempvalue)+";").toLatin1().data());//加速阶段终(匀速)速度（mm/s）

        tcp->tcpQueue.enqueue("COMMAND:STOP;");//停止
    }
}

//自动step走开始按钮
void motorizedstage::autoRunButtonSlot()
{
    //read all paramters   
    startPosition=tcp->pmc400Status.PMC400_Y_PositionEncode/10.0f;//um
    distancePerStep=ui->distLineEdit->text().toDouble();//um
    stepNumber=ui->stepNumLineEdit->text().toInt();
    exposureTime=ui->timeLineEdit->text().toInt();
    stepCnt=0;   //计数变量
    aimPos=0;
    status=0;


    //速度设置为25um/s
    tcp->tcpQueue.enqueue("COMMAND:STARTSPEED Y0;");//初速度（mm/s）
    tcp->tcpQueue.enqueue("COMMAND:ACCSPEED Y160;");//加速度（mm/s2）
    tcp->tcpQueue.enqueue("COMMAND:FINALSPEED Y160;");//加速阶段终(匀速)速度（mm/s）

    QEventLoop loop;
    QTimer::singleShot(500, &loop, SLOT(quit()));//非阻塞延时500ms，确保台子速度已经更新
    loop.exec();


    ui->startPosLineEdit->setText(QString::number(startPosition,10,1));
    ui->autoTextBrowser->append("Auto motion start!");
    autoRunMode=1;


}


