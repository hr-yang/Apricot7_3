#include "fpga_setting.h"
#include "qt_settings_widgetdatacontrol.h"
#include <QDebug>
#include <QSettings>
#include <QCloseEvent>
#include <QFileDialog>
#include <QtCore/QtMath>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QGridLayout>
#include <QValueAxis>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLabel>

FPGA_setting::FPGA_setting(QWidget *parent) :
    QDialog(parent),
    serial(nullptr),
    settings(nullptr),
    settingsFile0(nullptr),
    settingsFile1(nullptr),
    settingsFile2(nullptr),
    settingsFile3(nullptr)
{
    this->setObjectName("FPGA_Setting");
    this->setWindowIcon(QIcon(":/myicons/icons/fpga.png"));
    this->setWindowTitle("FPGA Setting");

    mRecievedCnt=0;

    //界面布局

    //串口功能布局
    QHBoxLayout *comLayout = new QHBoxLayout();
    QLabel* pCOMLabel = new QLabel("COM:");
    mpPortComboBox    = new QComboBox();
    mpComPushButton   = new QPushButton("OpenCOM");
    connect(mpComPushButton,&QPushButton::clicked,this,&FPGA_setting::openComButtonSlot);

    comLayout->addWidget(pCOMLabel);
    comLayout->addWidget(mpPortComboBox);
    comLayout->addWidget(mpComPushButton);

    //Profile配置按钮布局
    QHBoxLayout *profileLayout = new QHBoxLayout();
    QPushButton* pReadProfile=new QPushButton("ReadProfile");
    QPushButton* pSaveProfile=new QPushButton("SaveProfile");
    connect(pReadProfile,&QPushButton::clicked,this,&FPGA_setting::readProfileSlot);
    connect(pSaveProfile,&QPushButton::clicked,this,&FPGA_setting::saveProfileSlot);

    profileLayout->addWidget(pReadProfile);
    profileLayout->addWidget(pSaveProfile);


    //Profile Radio按钮布局
    QVBoxLayout *profileRadioVLayout = new QVBoxLayout();
    //Profile1、2 3、4切换按钮组
    {
        mpProfile1RadioButton=new QRadioButton("Profile 1");
        mpProfile2RadioButton=new QRadioButton("Profile 2");
        mpProfile3RadioButton=new QRadioButton("Profile 3");
        mpProfile4RadioButton=new QRadioButton("Profile 4");
        //防止配置按钮本身进入配置表存储
        mpProfile1RadioButton->setObjectName(nullptr);
        mpProfile2RadioButton->setObjectName(nullptr);
        mpProfile3RadioButton->setObjectName(nullptr);
        mpProfile4RadioButton->setObjectName(nullptr);

        //RadioButton组设置
        mpProfileButtonGroup=new QButtonGroup();
        mpProfileButtonGroup->addButton(mpProfile1RadioButton,0);
        mpProfileButtonGroup->addButton(mpProfile2RadioButton,1);
        mpProfileButtonGroup->addButton(mpProfile3RadioButton,2);
        mpProfileButtonGroup->addButton(mpProfile4RadioButton,3);
        mpProfile1RadioButton->setChecked(true);  //设置初选项

        connect(mpProfile1RadioButton, &QRadioButton::clicked, this, &FPGA_setting::profileModeButtonsSlot);
        connect(mpProfile2RadioButton, &QRadioButton::clicked, this, &FPGA_setting::profileModeButtonsSlot);
        connect(mpProfile3RadioButton, &QRadioButton::clicked, this, &FPGA_setting::profileModeButtonsSlot);
        connect(mpProfile4RadioButton, &QRadioButton::clicked, this, &FPGA_setting::profileModeButtonsSlot);

        profileRadioVLayout->addWidget(mpProfile1RadioButton);
        profileRadioVLayout->addWidget(mpProfile2RadioButton);
        profileRadioVLayout->addWidget(mpProfile3RadioButton);
        profileRadioVLayout->addWidget(mpProfile4RadioButton);
    }

    //TrigTest SWAM系统步进台扫描功能
    QHBoxLayout *horizontalLayout_4 = new QHBoxLayout();
    QLabel* pTestLabel=new QLabel("ExpTime:");
    mpExpTimeLineEdit=new QLineEdit(this);
    mpExpTimeLineEdit->setObjectName("ExpTimeLineEdit");
    QLabel* pUnitLabel=new QLabel("ms");
    QPushButton* pCameraTrigPushButton=new QPushButton("TrigTest");
    connect(pCameraTrigPushButton,&QPushButton::clicked,this,&FPGA_setting::cameraTrigTestSlot);

    horizontalLayout_4->addWidget(pTestLabel);
    horizontalLayout_4->addWidget(mpExpTimeLineEdit);
    horizontalLayout_4->addWidget(pUnitLabel);
    horizontalLayout_4->addWidget(pCameraTrigPushButton);

    //其他功能
    QPushButton* pSwitchPushButton=new QPushButton("Switch");
    connect(pSwitchPushButton,&QPushButton::clicked,this,&FPGA_setting::OnShowSwitchDialog);

    QPushButton* pGratingPushButton=new QPushButton("Grating");
    connect(pGratingPushButton,&QPushButton::clicked,this,&FPGA_setting::showGratingDialogSlot);

    QPushButton* pSIMPushButton=new QPushButton("SIM");
    connect(pSIMPushButton,&QPushButton::clicked,this,&FPGA_setting::showSIMDialogSlot);

    QPushButton* pChartPushButton=new QPushButton("Chart");
    connect(pChartPushButton,&QPushButton::clicked,this,&FPGA_setting::showChartSlot);

    QPushButton* pMolisPushButton=new QPushButton("MoLiS");
    connect(pMolisPushButton,&QPushButton::clicked,this,&FPGA_setting::showMolisDialogSlot);

    molisDialog=new MolisDialog(this);

    QPushButton* pResetFPGAPushButton=new QPushButton("ResetFPGA");
    pResetFPGAPushButton->setStyleSheet("color:red");
    connect(pResetFPGAPushButton,&QPushButton::clicked,this,&FPGA_setting::sendSoftReset);


    QPushButton* pSetParametersPushButton=new QPushButton("SetParameters");
    connect(pSetParametersPushButton,&QPushButton::clicked,this,&FPGA_setting::setSynSlot);


    QPushButton* pSynPushButton=new QPushButton("Syn");
    connect(pSynPushButton,&QPushButton::clicked,this,&FPGA_setting::synSlot);

    mpRecievedCounter=new QLabel("Recieved:"+QString::number(mRecievedCnt));


    //界面上方第一行布局
    QHBoxLayout *horizontalLayout_tool = new QHBoxLayout;
    horizontalLayout_tool->addLayout(comLayout);
    horizontalLayout_tool->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    horizontalLayout_tool->addLayout(profileLayout);
    horizontalLayout_tool->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    horizontalLayout_tool->addLayout(horizontalLayout_4);
    horizontalLayout_tool->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    horizontalLayout_tool->addWidget(pSwitchPushButton);
    horizontalLayout_tool->addWidget(pGratingPushButton);
    horizontalLayout_tool->addWidget(pSIMPushButton);
    horizontalLayout_tool->addWidget(pChartPushButton);
    horizontalLayout_tool->addWidget(pMolisPushButton);
    horizontalLayout_tool->addWidget(pResetFPGAPushButton);
    horizontalLayout_tool->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    horizontalLayout_tool->addWidget(pSetParametersPushButton);
    horizontalLayout_tool->addWidget(pSynPushButton);
    horizontalLayout_tool->addWidget(mpRecievedCounter);

    TTL_channel0 = new FPGA_TTL(this,0);
    TTL_channel1 = new FPGA_TTL(this,1);
    TTL_channel2 = new FPGA_TTL(this,2);
    TTL_channel3 = new FPGA_TTL(this,3);
    TTL_channel4 = new FPGA_TTL(this,4);
    TTL_channel5 = new FPGA_TTL(this,5);

    DA_channel0 = new fpgachannel(this,0);
    DA_channel1 = new fpgachannel(this,1);

    connect(DA_channel0,&fpgachannel::send_DA_Sgn,this,&FPGA_setting::send_DA_Paramters);
    connect(DA_channel1,&fpgachannel::send_DA_Sgn,this,&FPGA_setting::send_DA_Paramters);

    connect(DA_channel0,&fpgachannel::send_BRAM_DA_Sgn,this,&FPGA_setting::send_BRAM_DA_Data);
    connect(DA_channel1,&fpgachannel::send_BRAM_DA_Sgn,this,&FPGA_setting::send_BRAM_DA_Data);

    connect(DA_channel0,&fpgachannel::sendSoftTrigSgn,this,&FPGA_setting::sendSoftTrigParamters);
    connect(DA_channel1,&fpgachannel::sendSoftTrigSgn,this,&FPGA_setting::sendSoftTrigParamters);

    connect(TTL_channel0,&FPGA_TTL::send_TTL_ParamtersSgn,this,&FPGA_setting::send_TTL_Paramters);
    connect(TTL_channel1,&FPGA_TTL::send_TTL_ParamtersSgn,this,&FPGA_setting::send_TTL_Paramters);
    connect(TTL_channel2,&FPGA_TTL::send_TTL_ParamtersSgn,this,&FPGA_setting::send_TTL_Paramters);
    connect(TTL_channel3,&FPGA_TTL::send_TTL_ParamtersSgn,this,&FPGA_setting::send_TTL_Paramters);
    connect(TTL_channel4,&FPGA_TTL::send_TTL_ParamtersSgn,this,&FPGA_setting::send_TTL_Paramters);
    connect(TTL_channel5,&FPGA_TTL::send_TTL_ParamtersSgn,this,&FPGA_setting::send_TTL_Paramters);

    connect(TTL_channel0,&FPGA_TTL::sendSoftTrigSgn,this,&FPGA_setting::sendSoftTrigParamters);
    connect(TTL_channel1,&FPGA_TTL::sendSoftTrigSgn,this,&FPGA_setting::sendSoftTrigParamters);
    connect(TTL_channel2,&FPGA_TTL::sendSoftTrigSgn,this,&FPGA_setting::sendSoftTrigParamters);
    connect(TTL_channel3,&FPGA_TTL::sendSoftTrigSgn,this,&FPGA_setting::sendSoftTrigParamters);
    connect(TTL_channel4,&FPGA_TTL::sendSoftTrigSgn,this,&FPGA_setting::sendSoftTrigParamters);
    connect(TTL_channel5,&FPGA_TTL::sendSoftTrigSgn,this,&FPGA_setting::sendSoftTrigParamters);


    //主页面布局
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addLayout(horizontalLayout_tool, 0, 0, 1, 4);
    gridLayout->addLayout(profileRadioVLayout, 1, 0, 1, 4);

    gridLayout->addWidget(TTL_channel0,  2, 0, 1, 1);
    gridLayout->addWidget(TTL_channel1,  2, 1, 1, 1);
    gridLayout->addWidget(TTL_channel2,  2, 2, 1, 1);
    gridLayout->addWidget(TTL_channel3,  3, 0, 1, 1);
    gridLayout->addWidget(TTL_channel4,  3, 1, 1, 1);
    gridLayout->addWidget(TTL_channel5,  3, 2, 1, 1);
    gridLayout->addWidget(DA_channel0,   2, 3, 1, 1);
    gridLayout->addWidget(DA_channel1,   3, 3, 1, 1);

    this->setLayout(gridLayout);

    /*************************** Switch Dialog 子界面设置 **************************************************/
    switchDialog = new QDialog(this);
    switchDialog->setObjectName("SwitchDialog");
    switchDialog->setWindowTitle("Switch");

    QVBoxLayout* pSwitchVBoxLayout=new QVBoxLayout();


    QLabel *pAttentionLabel=new QLabel("Only for TTL Level,or damage FPGA!");
    pAttentionLabel->setStyleSheet("background-color:rgb(250,0,0);font-size:20px;color:blue");
    pSwitchVBoxLayout->addWidget(pAttentionLabel);

    QHBoxLayout* pS1HBoxLayout=new QHBoxLayout();
    QLabel* pS1Label=new QLabel("1 INs->2 OUTs:");
    pS1HBoxLayout->addWidget(pS1Label);
    {//Pos 和 Neg 切换按钮组
            QRadioButton* pOut1RadioButton=new QRadioButton("OUT 1");
            pOut1RadioButton->setObjectName("Out1RadioButton");       //只有在Qt_Settings_WidgetDataControl类中写的控件需要对象名
            QRadioButton* pOut2RadioButton=new QRadioButton("OUT 2");
            pOut2RadioButton->setObjectName("Out2RadioButton");
            //RadioButton组设置
            mpSwitch1ButtonGroup=new QButtonGroup();
            mpSwitch1ButtonGroup->addButton(pOut1RadioButton,0);
            mpSwitch1ButtonGroup->addButton(pOut2RadioButton,1);
            pOut1RadioButton->setChecked(true);  //设置初选项

    pS1HBoxLayout->addWidget(pOut1RadioButton);
    pS1HBoxLayout->addWidget(pOut2RadioButton);
    }
    pSwitchVBoxLayout->addLayout(pS1HBoxLayout);


    QHBoxLayout* pS2HBoxLayout=new QHBoxLayout();
    QLabel* pS2Label=new QLabel("2 INs->1 OUTs:");
    pS2HBoxLayout->addWidget(pS2Label);
    {//Pos 和 Neg 切换按钮组
            QRadioButton* pIn1RadioButton=new QRadioButton("IN 1");
            pIn1RadioButton->setObjectName("In1RadioButton");       //只有在Qt_Settings_WidgetDataControl类中写的控件需要对象名
            QRadioButton* pIn2RadioButton=new QRadioButton("IN 2");
            pIn2RadioButton->setObjectName("In2RadioButton");
            //RadioButton组设置
            mpSwitch2ButtonGroup=new QButtonGroup();
            mpSwitch2ButtonGroup->addButton(pIn1RadioButton,0);
            mpSwitch2ButtonGroup->addButton(pIn2RadioButton,1);
            pIn1RadioButton->setChecked(true);  //设置初选项

    pS2HBoxLayout->addWidget(pIn1RadioButton);
    pS2HBoxLayout->addWidget(pIn2RadioButton);
    }
    pSwitchVBoxLayout->addLayout(pS2HBoxLayout);

    QPushButton* pSwitchSendPushButton=new QPushButton("Send");
    connect(pSwitchSendPushButton,&QPushButton::clicked,this,&FPGA_setting::OnSendSwitchData);
    pSwitchVBoxLayout->addWidget(pSwitchSendPushButton);
    switchDialog->setLayout(pSwitchVBoxLayout);

    /*******************************  SIM Dialog 子界面设置  *************************************************/
    simDialog = new QDialog(this);
    simDialog->setObjectName("SIMdialog");//必须设置对象名称才能存入文件，否则为空
    simDialog->setWindowTitle("SIM");

    QVBoxLayout *pSIMVBoxLayout=new QVBoxLayout();

    QHBoxLayout* p1HBoxLayout=new QHBoxLayout();
    QLabel* pTrigEdgeLabel=new QLabel("TrigEdge:");
    p1HBoxLayout->addWidget(pTrigEdgeLabel);
    {//Pos 和 Neg 切换按钮组
            QRadioButton* pPosRadioButton=new QRadioButton("Pos");
            pPosRadioButton->setObjectName("PosRadioButton");       //只有在Qt_Settings_WidgetDataControl类中写的控件需要对象名
            QRadioButton* pNegRadioButton=new QRadioButton("Neg");
            pNegRadioButton->setObjectName("NegRadioButton");
            //RadioButton组设置
            mpTrigEdgeButtonGroup=new QButtonGroup();
            mpTrigEdgeButtonGroup->addButton(pPosRadioButton,0);
            mpTrigEdgeButtonGroup->addButton(pNegRadioButton,1);
            pPosRadioButton->setChecked(true);  //设置初选项

    p1HBoxLayout->addWidget(pPosRadioButton);
    p1HBoxLayout->addWidget(pNegRadioButton);
    }
    pSIMVBoxLayout->addLayout(p1HBoxLayout);


    QHBoxLayout* p2HBoxLayout=new QHBoxLayout();
    QLabel* pLabel2=new QLabel("TrigCont:");
    mpTrigCountSpinBox=new QSpinBox();
    mpTrigCountSpinBox->setObjectName("TrigCount");
    p2HBoxLayout->addWidget(pLabel2);
    p2HBoxLayout->addWidget(mpTrigCountSpinBox);
    pSIMVBoxLayout->addLayout(p2HBoxLayout);


    QHBoxLayout* p3HBoxLayout=new QHBoxLayout();
    QLabel* pLabel3=new QLabel("N Sequence:");
    mpN_SequenceSpinBox=new QSpinBox();
    mpN_SequenceSpinBox->setObjectName("N_Sequence");
    p3HBoxLayout->addWidget(pLabel3);
    p3HBoxLayout->addWidget(mpN_SequenceSpinBox);
    pSIMVBoxLayout->addLayout(p3HBoxLayout);

    mpLaser405CheckBox=new QCheckBox("405nm");
    mpLaser405CheckBox->setObjectName("405nm");
    mpLaser488CheckBox=new QCheckBox("488nm");
    mpLaser488CheckBox->setObjectName("488");
    mpLaser561CheckBox=new QCheckBox("561nm");
    mpLaser561CheckBox->setObjectName("561nm");
    mpLaser640CheckBox=new QCheckBox("640nm");
    mpLaser640CheckBox->setObjectName("640nm");

    QPushButton* pSIMSendPushButton=new QPushButton("Send");
    connect(pSIMSendPushButton,&QPushButton::clicked,this,&FPGA_setting::sendSIMChannelSlot);

    pSIMVBoxLayout->addWidget(mpLaser405CheckBox);
    pSIMVBoxLayout->addWidget(mpLaser488CheckBox);
    pSIMVBoxLayout->addWidget(mpLaser561CheckBox);
    pSIMVBoxLayout->addWidget(mpLaser640CheckBox);//一个Laout多次添加同一个部件会打不开界面
    pSIMVBoxLayout->addWidget(pSIMSendPushButton);
    simDialog->setLayout(pSIMVBoxLayout);

    /****************************   gratingDialog  子界面设置  ******************************************/
    gratingDialog = new QDialog(this);
    gratingDialog->setWindowTitle("Grating");
    gratingDialog->setObjectName("gratingDialog");//必须设置对象名称才能存入文件，否则为空

    QVBoxLayout * pMainLayout=new QVBoxLayout();


    QHBoxLayout* pHBoxLayout=new QHBoxLayout();
    QLabel* pGratingLabel=new QLabel("Grating Value:");
    pGratingLabel->setAlignment(Qt::AlignCenter);
    pGratingLabel->setMinimumSize(100,30);
    pGratingLabel->setObjectName("pGratingLabel");
    mpGratingLineEdit=new QLineEdit();//显示光栅尺数值
    mpGratingLineEdit->setReadOnly(true);
    mpGratingLineEdit->setObjectName("pGratingLineEdit");
    pHBoxLayout->addWidget(pGratingLabel);
    pHBoxLayout->addWidget(mpGratingLineEdit);
    pMainLayout->addLayout(pHBoxLayout);

    QGroupBox *pGratingGroupBox=new QGroupBox("Table");
    pGratingGroupBox->setObjectName("GratingGroupBox");
    {

        QVBoxLayout * pVBoxLayout=new QVBoxLayout();
        mpGratingTable = new Table(gratingDialog);
        mpGratingTable->setObjectName("Grating_Table");
        pVBoxLayout->addWidget(mpGratingTable);
        pGratingGroupBox->setLayout(pVBoxLayout);
    }
    pMainLayout->addWidget(pGratingGroupBox);

    QHBoxLayout* pHBoxLayout2=new QHBoxLayout();
    QPushButton* pSendButton=new QPushButton("Send Table");
    connect(pSendButton,&QPushButton::clicked,this,&FPGA_setting::OnSendGratingTable);
    pHBoxLayout2->addSpacerItem(new QSpacerItem(48, 24, QSizePolicy::Expanding, QSizePolicy::Minimum));
    pHBoxLayout2->addWidget(pSendButton);
    pMainLayout->addLayout(pHBoxLayout2);


    QHBoxLayout* pStart1HBoxLayout=new QHBoxLayout();
    QLabel* pInitLabel=new QLabel("InitPos:");
    mpGratingInitPos=new QLineEdit();
    mpGratingInitPos->setObjectName("GratingInitPos");
    pStart1HBoxLayout->addWidget(pInitLabel);
    pStart1HBoxLayout->addWidget(mpGratingInitPos);
    pStart1HBoxLayout->addSpacerItem(new QSpacerItem(48,24,QSizePolicy::Expanding));
    pMainLayout->addLayout(pStart1HBoxLayout);


    QHBoxLayout* pStart2HBoxLayout=new QHBoxLayout();
    QPushButton* pStartPushButton=new QPushButton("StartTrig");
    connect(pStartPushButton,&QPushButton::clicked,this,&FPGA_setting::OnSendStartGratingTrig);
    QLabel* pTrigEN=new QLabel("TrigOn:");
    mpGratingTrigEn= new SwitchButton(gratingDialog);


    pStart2HBoxLayout->addWidget(pTrigEN);
    pStart2HBoxLayout->addWidget(mpGratingTrigEn);
    pStart2HBoxLayout->addSpacerItem(new QSpacerItem(QSizePolicy::Expanding,QSizePolicy::Expanding));
    pStart2HBoxLayout->addWidget(pStartPushButton);
    pMainLayout->addLayout(pStart2HBoxLayout);

    gratingDialog->setLayout(pMainLayout);

    /****************************Chart Dialog ******************************************/
//    mychartdata   = new MyChartData;
//    connect(channel0,&fpgachannel::sendSgn,mychartdata,&MyChartData::chartDataProcessSlot);
//    connect(mychartdata,&MyChartData::addseries,this,&FPGA_setting::addSeries);
//    mychartdata->moveToThread(thread);
//    thread->start();


    //查找可用的串口
    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            mpPortComboBox->addItem(serial.portName());
            serial.close();
        }
    }


    //创建和读取一样  必须在save之后磁盘才会有该文件
    settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/ApricotConfig.ini", QSettings::IniFormat,this);
    settings->setIniCodec("UTF-8");

    Qt_Settings_WidgetDataControl::read(settings,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);

    //恢复设置使能
    mpPortComboBox->setEnabled(true);
    mpComPushButton->setText(tr("OpenCOM"));

    isClose=1;//串口关闭标志

    setAttribute(Qt::WA_DeleteOnClose);//解决办法   因为正常情况下当关闭子窗体时不会调用析构函数，只有关闭父窗体时，才会调用子窗体的析构函数。

    qDebug()<<"fpga_setting thread id: "<<QThread::currentThreadId();
}

FPGA_setting::~FPGA_setting()
{
    Qt_Settings_WidgetDataControl::save(settings,this, Qt_Settings_WidgetDataControl::controlFlgasWidgets);

    if(isClose==0)//串口未关闭
    {
        serial->clear();
        serial->close();
        serial->deleteLater();
    }

//    thread->quit();
//    thread->wait();//必须等线程结束才能退出，不然会死机
//    delete mychartdata;
//    delete thread;
//    delete chartWidget;
}


void FPGA_setting::closeEvent(QCloseEvent * event)
{
    emit sendCloseSgn("fpga_setting");//发送注销子窗口句柄信号
    event->accept();
}

// 获取上次保存的路径或提供一个默认路径
QString FPGA_setting::getLastOpenPath()
{
    return settings->value("lastOpenPath").toString();
}

// 保存路径到设置
void FPGA_setting::saveLastOpenPath(const QString& path)
{
    settings->setValue("lastOpenPath", path);
}

void FPGA_setting::showSIMDialogSlot()
{
    simDialog->show();
}

void FPGA_setting::OnShowSwitchDialog()
{
    switchDialog->show();
}

void FPGA_setting::showGratingDialogSlot()
{
    gratingDialog->show();
}



void FPGA_setting::openComButtonSlot()
{
    if(mpComPushButton->text() == tr("OpenCOM"))
    {
        serial = new QSerialPort;
        //设置串口名
        serial->setPortName(mpPortComboBox->currentText());
        //打开串口
        serial->open(QIODevice::ReadWrite);
        //设置波特率
        serial->setBaudRate(QSerialPort::Baud115200);
        //设置数据位数
        serial->setDataBits(QSerialPort::Data8);//设置数据位8
        //设置校验位
        serial->setParity(QSerialPort::NoParity);
        //设置停止位
        serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
        //设置流控制
        serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

        //关闭设置菜单使能
        mpPortComboBox->setEnabled(false);
        mpComPushButton->setText(tr("CloseCOM"));

        //只有打开了串口才连接这些跟串口有关的信号槽,防止串口未打开的时候按下程序崩溃!!!
        connect(serial,&QSerialPort::readyRead,this,&FPGA_setting::readSerialDataSlot);

        isClose=0;//串口打开标志
    }
    else
    {
        //关闭串口
        serial->clear();
        serial->close();
        serial->deleteLater();

        isClose=1;//串口关闭标志

        //恢复设置使能
        mpPortComboBox->setEnabled(true);
        mpComPushButton->setText(tr("OpenCOM"));

    }
}




//读取配置文件
void FPGA_setting::readProfileSlot()
{

    QString lastPath = getLastOpenPath();
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("Open profile"),
        lastPath,
        tr("Profile Files (*.ini);"));

    if (!filePath.isEmpty())
    {
        saveLastOpenPath(filePath); // 保存新的路径
    }

    if (!filePath.isNull())
    {
        QString fileName = QFileInfo(filePath).fileName();

        if(mpProfileButtonGroup->checkedId()==0)
        {
           if(settingsFile0!=nullptr)//先删除指针
           {
               delete settingsFile0;
           }
           settingsFile0 = new QSettings(filePath, QSettings::IniFormat,this);//必须在save之后磁盘才会有该文件
           settingsFile0->setIniCodec("UTF-8");
           Qt_Settings_WidgetDataControl::read(settingsFile0,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
           mpProfile1RadioButton->setText(fileName);
        }
        else if(mpProfileButtonGroup->checkedId()==1)
        {
           if(settingsFile1!=nullptr)//先删除指针
           {
               delete settingsFile1;
           }
           settingsFile1 = new QSettings(filePath, QSettings::IniFormat,this);//必须在save之后磁盘才会有该文件
           settingsFile1->setIniCodec("UTF-8");
           Qt_Settings_WidgetDataControl::read(settingsFile1,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
           mpProfile2RadioButton->setText(fileName);
        }
        else if(mpProfileButtonGroup->checkedId()==2)
        {
           if(settingsFile2!=nullptr)//先删除指针
           {
               delete settingsFile2;
           }
           settingsFile2 = new QSettings(filePath, QSettings::IniFormat,this);//必须在save之后磁盘才会有该文件
           settingsFile2->setIniCodec("UTF-8");
           Qt_Settings_WidgetDataControl::read(settingsFile2,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
           mpProfile3RadioButton->setText(fileName);
        }
        else if(mpProfileButtonGroup->checkedId()==3)
        {
           if(settingsFile3!=nullptr)//先删除指针
           {
               delete settingsFile3;
           }
           settingsFile3 = new QSettings(filePath, QSettings::IniFormat,this);//必须在save之后磁盘才会有该文件
           settingsFile3->setIniCodec("UTF-8");
           Qt_Settings_WidgetDataControl::read(settingsFile3,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
           mpProfile4RadioButton->setText(fileName);
        }
     }
}

void FPGA_setting::saveProfileSlot()
{

    QString lastPath = getLastOpenPath();
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("Save Profile"),
        lastPath,
        tr("Profile Files (*.ini)"));

    if (!filePath.isEmpty())
    {
        saveLastOpenPath(filePath); // 保存新的路径
    }

    if (!filePath.isNull())
    {
        QString fileName = QFileInfo(filePath).fileName();

        if(mpProfileButtonGroup->checkedId()==0)
        {
            if(settingsFile0!=nullptr)
            {
                delete settingsFile0;
            }

            settingsFile0 = new QSettings(filePath, QSettings::IniFormat,this);//必须在save之后磁盘才会有该文件
            settingsFile0->setIniCodec("UTF-8");

            Qt_Settings_WidgetDataControl::save(settingsFile0,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
            mpProfile1RadioButton->setText(fileName);
        }
        else if(mpProfileButtonGroup->checkedId()==1)
        {
            if(settingsFile1!=nullptr)
            {
                delete settingsFile1;
            }

            settingsFile1 = new QSettings(filePath, QSettings::IniFormat,this);//必须在save之后磁盘才会有该文件
            settingsFile1->setIniCodec("UTF-8");

            Qt_Settings_WidgetDataControl::save(settingsFile1,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
            mpProfile2RadioButton->setText(fileName);
        }
        else if(mpProfileButtonGroup->checkedId()==2)
        {
            if(settingsFile2!=nullptr)
            {
                delete settingsFile2;
            }

            settingsFile2 = new QSettings(filePath, QSettings::IniFormat,this);//必须在save之后磁盘才会有该文件
            settingsFile2->setIniCodec("UTF-8");

            Qt_Settings_WidgetDataControl::save(settingsFile2,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
            mpProfile3RadioButton->setText(fileName);
        }
        else if(mpProfileButtonGroup->checkedId()==3)
        {
            if(settingsFile3!=nullptr)
            {
                delete settingsFile3;
            }

            settingsFile3 = new QSettings(filePath, QSettings::IniFormat,this);//必须在save之后磁盘才会有该文件
            settingsFile3->setIniCodec("UTF-8");

            Qt_Settings_WidgetDataControl::save(settingsFile3,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
            mpProfile4RadioButton->setText(fileName);
        }
    }
}

void FPGA_setting::profileModeButtonsSlot()
{
    switch(mpProfileButtonGroup->checkedId())
    {
    case 0:
        if(settingsFile0!=nullptr)
            Qt_Settings_WidgetDataControl::read(settingsFile0,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
        break;
    case 1:
        if(settingsFile1!=nullptr)
            Qt_Settings_WidgetDataControl::read(settingsFile1,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
        break;
    case 2:
        if(settingsFile2!=nullptr)
            Qt_Settings_WidgetDataControl::read(settingsFile2,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
        break;
    case 3:
        if(settingsFile3!=nullptr)
            Qt_Settings_WidgetDataControl::read(settingsFile3,this,Qt_Settings_WidgetDataControl::controlFlgasWidgets);
        break;
    }
}








void FPGA_setting::cameraTrigTestSlot()
{

//   if(ui->lineEdit_ExpTime->text().toFloat()!=0)
//   {
//           float freq=500/ui->lineEdit_ExpTime->text().toFloat();//因为50%占空比
//           fpgaSendProcessSlot(6,7,1,1,0,0,freq,0,50);
//   }

}






void FPGA_setting::sendSIMChannelSlot()
{
    uchar trig_edge   = mpTrigEdgeButtonGroup->checkedId();
    uchar trig_count  = mpTrigCountSpinBox->value();

    uchar op_channel=0;
    if(mpLaser405CheckBox->isChecked())   op_channel|=0x01;
    if(mpLaser488CheckBox->isChecked())   op_channel|=0x02;
    if(mpLaser561CheckBox->isChecked())   op_channel|=0x04;
    if(mpLaser640CheckBox->isChecked())   op_channel|=0x08;

    uchar n_sequence = mpN_SequenceSpinBox->value();

    sendSIMChannel(trig_edge,trig_count,op_channel,n_sequence);
}



void FPGA_setting::OnSendSwitchData()
{
    uchar s1  = mpSwitch1ButtonGroup->checkedId();
    uchar s2  = mpSwitch2ButtonGroup->checkedId();

    sendSwitchData(s1, s2);
}



static int32_t GratingPosition[26];
//发送光栅尺位置数据
void FPGA_setting::OnSendGratingTable()
{
    //需要发送2列,第0列为符号0/1，第1列为值

    //第0列  实际表格是从0开始编号
    uint32_t trig_sign=0;
    for(int i=0; i<mpGratingTable->p_table->rowCount(); i++)//行必须从0开始
    {
            if(mpGratingTable->p_table->item(i, 0)!=nullptr)//如果TableWidget中内容为空的时候访问数据,则会造成卡死！所以先判断文本是否为空.
            {
                QString  temp = mpGratingTable->p_table->item(i,0)->text();
                uint32_t sign_temp=temp.toUInt();
                trig_sign=trig_sign|sign_temp<<i;
            }
    }

    //第1列
    for(int i=0; i<mpGratingTable->p_table->rowCount(); i++)//行必须从0开始
    {
            if(mpGratingTable->p_table->item(i, 1)!=nullptr)//如果TableWidget中内容为空的时候访问数据,则会造成卡死！所以先判断文本是否为空.
            {
                QString  temp = mpGratingTable->p_table->item(i,1)->text();
                int32_t  value=   temp.toInt();
                GratingPosition[i]=value;
            }
            else
                GratingPosition[i]=0;
    }

    uchar table_len = mpGratingTable->p_table->rowCount();
    qDebug("trig_sign:%x",trig_sign);
    qDebug()<<table_len;
    sendGratingTable(trig_sign, table_len,GratingPosition);
}

void FPGA_setting::OnSendStartGratingTrig()
{
    int32_t init_pos=mpGratingInitPos->text().toInt();
    uchar trig_en=mpGratingTrigEn->getChecked();
    qDebug()<<"checked:"<<trig_en;
    sendStartGratingTrig(init_pos,trig_en );
}


void FPGA_setting::showChartSlot()
{

//    if(ui->pushButton_chart->text() == tr("ShowChart"))
//    {
//        ui->pushButton_chart->setText(tr("HideChart"));

//        chartWidget = new QWidget;//加this的话父窗口为本窗口，不能浮出
//        QChartView *chartView = new QChartView(m_chart, chartWidget); // Create chart view with the chart
//        chartView->setRenderHint(QPainter::Antialiasing);

//        QGridLayout * chartLayout = new QGridLayout(); // Create layout for grid and detached legend
//        chartLayout->addWidget(chartView, 0, 1, 3, 1);//chartview 必须使用Layout，不然会遮盖别的控件

//        chartWidget->setLayout(chartLayout);
//        chartWidget->show();
//        chartWidget->resize(1000,600);
//        addSeries();

//    }
//    else
//    {
//        ui->pushButton_chart->setText(tr("ShowChart"));
//        chartWidget->close();
//    }
}


void FPGA_setting::showMolisDialogSlot()
{
    molisDialog->show();
}


//void FPGA_setting::addSeries()
//{
//    QLineSeries *series = new QLineSeries();
//    m_series.append(series);
//    QValueAxis *axisX = new QValueAxis;
//         axisX->setRange(0, 10);    //设置范围
//         axisX->setLabelFormat("%u");   //设置刻度的格式
//         axisX->setGridLineVisible(true);   //网格线可见
//         axisX->setTickCount(10);       //设置多少格
//         axisX->setMinorTickCount(1);   //设置每格小刻度线的数目
//         setAxisX(axisX, series);       //为chart设置X轴,并把轴附加到series线上

//    QValueAxis *axisY = new QValueAxis;
//         axisY->setRange(0, 16383);
//         axisY->setLabelFormat("%u");
//         axisY->setGridLineVisible(true);
//         axisY->setTickCount(10);
//         axisY->setMinorTickCount(1);

//   series->setName(QString("line " + QString::number(m_series.count())));

//    // Make some sine wave for data
//    series->append(mychartdata->data);

//    m_chart->addSeries(series);

//    if (m_series.count() == 1)
//        m_chart->createDefaultAxes();
//}




void FPGA_setting::setSynSlot()//一键设置同步功能
{
    TTL_channel0->setSyn();
    QThread::msleep(10);
    TTL_channel1->setSyn();
    QThread::msleep(10);
    TTL_channel2->setSyn();
    QThread::msleep(10);
    TTL_channel3->setSyn();
    QThread::msleep(10);
    TTL_channel4->setSyn();
    QThread::msleep(10);
    TTL_channel5->setSyn();
    QThread::msleep(10);
    DA_channel0->setSyn();
    QThread::msleep(10);
    DA_channel1->setSyn();
}

void FPGA_setting::synSlot()
{
    sendSoftTrigParamters(8);//设置所有通道一起触发
}



//串口接收槽函数
void FPGA_setting::readSerialDataSlot()
{
    QByteArray buf;
    buf = serial->readAll();
    if(!buf.isEmpty())
    {
        for (int i = 0; i < buf.size(); ++i)
            DataReceivePrepare(buf.at(i));
    }
    buf.clear();
}

































/*******************************************发送到FPGA的底层函数******************************************************************************/

//数据拆分宏定义，在发送大于1字节的数据类型时，比如int16、float等，需要把数据拆分成单独字节进行发送
#define BYTE0(dwTemp)       ( *( (char *)(&dwTemp)		) )
#define BYTE1(dwTemp)       ( *( (char *)(&dwTemp) + 1) )
#define BYTE2(dwTemp)       ( *( (char *)(&dwTemp) + 2) )
#define BYTE3(dwTemp)       ( *( (char *)(&dwTemp) + 3) )

void FPGA_setting::send_TTL_Paramters(uchar axis,uchar trig_source,uchar trig_edge,uchar trig_count,
                       uint32_t period,uint32_t duty,uint32_t phase,uint32_t period_number,uchar mode)
{
    uchar _cnt=0;
    volatile uint32_t _temp;
    char data_to_send[50];	//发送数据缓存

    data_to_send[_cnt++]=0xFF;//帧头
    data_to_send[_cnt++]=0xFE;
    data_to_send[_cnt++]=0x01;//功能字
    data_to_send[_cnt++]=0;//长度，后面计算

    //一个完整word
    data_to_send[_cnt++]=axis;
    data_to_send[_cnt++]=trig_source;
    data_to_send[_cnt++]=trig_edge;
    data_to_send[_cnt++]=trig_count;

    _temp = period;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = duty;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = phase;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = period_number;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    data_to_send[_cnt++]=mode;

    data_to_send[3] = _cnt-4;

    uchar sum = 0;
    for(uchar i=0;i<_cnt;i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    if(isClose==0)//串口打开了
    {
        serial->write(data_to_send,_cnt);//要写长度参数
        serial->flush();
    }
}

void FPGA_setting::send_DA_Paramters(uchar axis,uchar trig_source,uchar trig_edge,uchar trig_count,
                                     uint32_t DA_in, uint32_t DA_min,uint32_t DA_max,
                                     uint32_t period,uint32_t duty,uint32_t phase,uint32_t step,uint32_t read_len,
                                     uint32_t period_number,uchar mode)
{
    uchar _cnt=0;
    volatile uint32_t _temp;
    char data_to_send[50];	//发送数据缓存

    data_to_send[_cnt++]=0xFF;//帧头
    data_to_send[_cnt++]=0xFE;
    data_to_send[_cnt++]=0x02;//功能字
    data_to_send[_cnt++]=0;//长度，后面计算

    //一个完整word
    data_to_send[_cnt++]=axis;
    data_to_send[_cnt++]=trig_source;
    data_to_send[_cnt++]=trig_edge;
    data_to_send[_cnt++]=trig_count;

    _temp = DA_in;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = DA_min;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = DA_max;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);


    _temp = period;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = duty;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = phase;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = step;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = read_len;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    _temp = period_number;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    data_to_send[_cnt++]=mode;

    data_to_send[3] = _cnt-4;

    uchar sum = 0;
    for(uchar i=0;i<_cnt;i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    if(isClose==0)//串口打开了
    {
        serial->write(data_to_send,_cnt);//要写长度参数
        serial->flush();
    }
}

void FPGA_setting::send_BRAM_DA_Data(uchar axis, uchar table_len,uint16_t *buffer)
{
    uchar _cnt=0;
    volatile uint16_t _temp;
    char data_to_send[500];	//发送数据缓存

    data_to_send[_cnt++]=0xFF;//帧头
    data_to_send[_cnt++]=0xFE;
    data_to_send[_cnt++]=0x03;//功能字
    data_to_send[_cnt++]=0;//长度，后面计算

    //一个完整uint16_t
    data_to_send[_cnt++]=axis;
    data_to_send[_cnt++]=table_len;

    for(uchar i=0;i<table_len;i++)
    {
        _temp = buffer[i];
        data_to_send[_cnt++]=BYTE0(_temp);
        data_to_send[_cnt++]=BYTE1(_temp);
    }

    data_to_send[3] = _cnt-4;

    uchar sum = 0;
    for(uchar i=0;i<_cnt;i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    if(isClose==0)//串口打开了
    {
        serial->write(data_to_send,_cnt);//要写长度参数
        serial->flush();
    }
}

void FPGA_setting::sendSoftTrigParamters(uchar order)
{
    uchar _cnt=0;
    char data_to_send[50];	//发送数据缓存

    data_to_send[_cnt++]=0xFF;//帧头
    data_to_send[_cnt++]=0xFE;
    data_to_send[_cnt++]=0x04;//功能字
    data_to_send[_cnt++]=0;//长度，后面计算

    //一个完整word
    data_to_send[_cnt++]=order;

    data_to_send[3] = _cnt-4;

    uchar sum = 0;
    for(uchar i=0;i<_cnt;i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    if(isClose==0)//串口打开了
    {
        serial->write(data_to_send,_cnt);//要写长度参数
        serial->flush();
    }
}

void FPGA_setting::sendSIMChannel(uchar trig_edge, uchar trig_counter, uchar op_channel,uchar n_sequence)
{
    uchar _cnt=0;
    char data_to_send[50];	//发送数据缓存

    data_to_send[_cnt++]=0xFF;//帧头
    data_to_send[_cnt++]=0xFE;
    data_to_send[_cnt++]=0x05;//功能字
    data_to_send[_cnt++]=0;//长度，后面计算

    //一个完整word
    data_to_send[_cnt++]=trig_edge;
    data_to_send[_cnt++]=trig_counter;
    data_to_send[_cnt++]=op_channel;
    data_to_send[_cnt++]=n_sequence;

    data_to_send[3] = _cnt-4;

    uchar sum = 0;
    for(uchar i=0;i<_cnt;i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    if(isClose==0)//串口打开了
    {
        serial->write(data_to_send,_cnt);//要写长度参数
        serial->flush();
    }
}



void FPGA_setting::sendSoftReset(void)
{
    uchar _cnt=0;
    char data_to_send[50];	//发送数据缓存

    data_to_send[_cnt++]=0xFF;//帧头
    data_to_send[_cnt++]=0xFE;
    data_to_send[_cnt++]=0x06;//功能字
    data_to_send[_cnt++]=0;//长度，后面计算

    //一个完整word
    data_to_send[_cnt++]=0x00;

    data_to_send[3] = _cnt-4;

    uchar sum = 0;
    for(uchar i=0;i<_cnt;i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    if(isClose==0)//串口打开了
    {
        serial->write(data_to_send,_cnt);//要写长度参数
        serial->flush();
    }
}


void FPGA_setting::sendGratingTable(uint32_t trig_sign, uchar trig_number,int32_t *buffer)
{
    uchar _cnt=0;
    volatile int32_t _temp;
    char data_to_send[500];	//发送数据缓存

        data_to_send[_cnt++]=0xFF;//帧头
        data_to_send[_cnt++]=0xFE;
        data_to_send[_cnt++]=0x07;//功能字
        data_to_send[_cnt++]=0;//长度，后面计算

        //一个完整32bit
        data_to_send[_cnt++]=trig_number;
        data_to_send[_cnt++]=0;
        data_to_send[_cnt++]=0;
        data_to_send[_cnt++]=0;

        data_to_send[_cnt++]=BYTE0(trig_sign);
        data_to_send[_cnt++]=BYTE1(trig_sign);
        data_to_send[_cnt++]=BYTE2(trig_sign);
        data_to_send[_cnt++]=BYTE3(trig_sign);

        for(uchar i=0;i<trig_number;i++)
        {
            _temp = buffer[i];
            data_to_send[_cnt++]=BYTE0(_temp);
            data_to_send[_cnt++]=BYTE1(_temp);
            data_to_send[_cnt++]=BYTE2(_temp);
            data_to_send[_cnt++]=BYTE3(_temp);
        }

        data_to_send[3] = _cnt-4;

        uchar sum = 0;
        for(uchar i=0;i<_cnt;i++)
            sum += data_to_send[i];
        data_to_send[_cnt++] = sum;

        if(isClose==0)//串口打开了
        {
            serial->write(data_to_send,_cnt);//要写长度参数
            serial->flush();
        }
}


void FPGA_setting::sendStartGratingTrig(int32_t init_pos, uchar trig_en)
{
    uchar _cnt=0;
    volatile int32_t _temp;
    char data_to_send[50];	//发送数据缓存

    data_to_send[_cnt++]=0xFF;//帧头
    data_to_send[_cnt++]=0xFE;
    data_to_send[_cnt++]=0x08;//功能字
    data_to_send[_cnt++]=0;//长度，后面计算

    _temp = init_pos;
    data_to_send[_cnt++]=BYTE0(_temp);
    data_to_send[_cnt++]=BYTE1(_temp);
    data_to_send[_cnt++]=BYTE2(_temp);
    data_to_send[_cnt++]=BYTE3(_temp);

    data_to_send[_cnt++]=trig_en;

    data_to_send[3] = _cnt-4;

    uchar sum = 0;
    for(uchar i=0;i<_cnt;i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    if(isClose==0)//串口打开了
    {
        serial->write(data_to_send,_cnt);//要写长度参数
        serial->flush();
    }
}


void FPGA_setting::sendSwitchData(uchar s1, uchar s2)
{
    uchar _cnt=0;
    char data_to_send[50];	//发送数据缓存

    data_to_send[_cnt++]=0xFF;//帧头
    data_to_send[_cnt++]=0xFE;
    data_to_send[_cnt++]=0x09;//功能字
    data_to_send[_cnt++]=0;//长度，后面计算

    //一个完整word
    data_to_send[_cnt++]=s1;
    data_to_send[_cnt++]=s2;

    data_to_send[3] = _cnt-4;

    uchar sum = 0;
    for(uchar i=0;i<_cnt;i++)
        sum += data_to_send[i];
    data_to_send[_cnt++] = sum;

    if(isClose==0)//串口打开了
    {
        serial->write(data_to_send,_cnt);//要写长度参数
        serial->flush();
    }
}



/***************************************串口接收FPGA的底层函数***************************************************************************************/


//将串口接收数据放入指定位置
void FPGA_setting::DataReceivePrepare(uchar data)
{
    static uchar RxBuffer[200];
    static uchar _data_len = 0,_data_cnt = 0;//数据位长度
    static uchar state = 0;
    if(state==0&&data==0xFF)//接收到帧头数据
    {
        state=1;
        RxBuffer[0]=data;
    }
    else if(state==1&&data==0xEE)//接收到帧头数据
    {
        state=2;
        RxBuffer[1]=data;
    }
    else if(state==2&&data<0XFF)//功能帧数据
    {
        state=3;
        RxBuffer[2]=data;
    }
    else if(state==3&&data<255)//这里的长度有限制！！！
    {
        state = 4;
        RxBuffer[3]=data;

        _data_len = data;//数据位长度
        _data_cnt = 0;
    }
    else if(state==4&&_data_len>0)
    {
        _data_len--;
        RxBuffer[4+_data_cnt++]=data;
        if(_data_len==0)
            state = 5;
    }
    else if(state==5)
    {
        state = 0;
        RxBuffer[4+_data_cnt]=data;//校验位
        DataAnalyze(RxBuffer,_data_cnt+5);//总长度因为从0开始，所以是+5
    }
    else
        state = 0;
}



//分析数据内容
void FPGA_setting::DataAnalyze(uchar *data_buf,uint16_t num)
{
    uchar sum = 0,i;
    for(i=0;i<(num-1);i++)
        sum += *(data_buf+i);
    if(!(sum==*(data_buf+num-1)))		return;//判断sum

    if(!(*(data_buf)==0xFF && *(data_buf+1)==0xEE))		return;		//判断帧头

    if(*(data_buf+2)==0X01)//应答帧
    {
        uchar ack;
        memcpy(&ack, data_buf+4,*(data_buf+3));//小端模式

        if(ack!=0)  mpRecievedCounter->setText("Recieved:"+QString::number(++mRecievedCnt));
        if(ack==0x03)  QMessageBox::information(nullptr, "WriteToFPGA", "SendSucess!", QMessageBox::Yes, QMessageBox::Yes);//发送FPGA DA表成功
    }
    if(*(data_buf+2)==0X02)//光栅尺数据
    {
        int32_t grating;
        memcpy(&grating, data_buf+4,*(data_buf+3));//小端模式 
        mpGratingLineEdit->setText(QString::number(grating));
    }
}



