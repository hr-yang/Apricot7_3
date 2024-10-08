#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fpga_setting.h"
#include "stimulate.h"
#include "nanostage.h"
#include "motorizedstage.h"
#include "laserctrl.h"
#include "kokidialog.h"
#include "aboutdialog.h"

#include <QDesktopWidget>
#include <QThread>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createActions();
    createMenus();
    createToolBars();

    //把主窗口移到屏幕的右上角
    QRect deskRect = QApplication::desktop()->availableGeometry();
    this->move(deskRect.right() - 600, 30);
    this->setFixedSize(340,80);



    // 指定文件：
    QFile inputFile(":/log/update_log.txt");
    // 只读打开：
    inputFile.open(QIODevice::ReadOnly);
    // 文本流：
    QTextStream in(&inputFile);
    // 将文本流读取到字符串中：
    QString line = in.readAll();
    // 关闭文本流：
    inputFile.close();

    int index=line.lastIndexOf(":");
    QString releaseNo=line.mid(index-3,3);//：号前3个字符


    this->setWindowTitle("Apricot "+releaseNo);

    qDebug()<<"mainwindow thread id: "<<QThread::currentThreadId();


    connect(&myProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(showFinished(int, QProcess::ExitStatus)));



}

MainWindow::~MainWindow()
{

    delete fpgaSettingAction;
    delete stimulateAction;
    delete motorizedAction;
    delete nanoAction;
    delete laserCtrlAction;
    delete sigmaCtrlAction;

    delete ui;
}

void MainWindow::createActions()
{
    fpgaSettingAction = new QAction(QIcon(":/myicons/icons/fpga.png"),tr("FPGA  Setting"), this);
    fpgaSettingAction->setShortcut(tr("Ctrl+F"));
    fpgaSettingAction->setStatusTip(tr("Open the FPGA setting window"));
    connect(fpgaSettingAction, SIGNAL(triggered()), this, SLOT(OpenFPGA_SettingSlot()));


    stimulateAction = new QAction(QIcon(":/myicons/icons/stimulate.png"),tr("Stimulate"), this);
    stimulateAction->setShortcut(tr("Ctrl+T"));
    stimulateAction->setStatusTip(tr("Open the stimulate window"));
    connect(stimulateAction, SIGNAL(triggered()), this, SLOT(OpenStimulateSlot()));

    motorizedAction = new QAction(QIcon(":/myicons/icons/motorized.png"),tr("Motorized Stage"), this);
    motorizedAction->setShortcut(tr("Ctrl+M"));
    motorizedAction->setStatusTip(tr("Open the motorized stage window"));
    connect(motorizedAction, SIGNAL(triggered()), this, SLOT(OpenMotorizedStageSlot()));

    nanoAction = new QAction(QIcon(":/myicons/icons/nano.png"),tr("Nano Stage"), this);
    nanoAction->setShortcut(tr("Ctrl+N"));
    nanoAction->setStatusTip(tr("Open the nano stage window"));
    connect(nanoAction, SIGNAL(triggered()), this, SLOT(OpenNanoStageSlot()));

    laserCtrlAction = new QAction(QIcon(":/myicons/icons/laser_ctrl.png"),tr("Laser Control"), this);
    laserCtrlAction->setShortcut(tr("Ctrl+L"));
    laserCtrlAction->setStatusTip(tr("Open the laser control window"));
    connect(laserCtrlAction, SIGNAL(triggered()), this, SLOT(OpenLaserCtrlSlot()));


    sigmaCtrlAction = new QAction(QIcon(":/myicons/icons/sigma.png"),tr("Sigma Control"), this);
    sigmaCtrlAction->setShortcut(tr("Ctrl+S"));
    sigmaCtrlAction->setStatusTip(tr("Open the sigma control window"));
    connect(sigmaCtrlAction, SIGNAL(triggered()), this, SLOT(OpenSigmaCtrlSlot()));

    sigmaCtrl2Action = new QAction(QIcon(":/myicons/icons/sigma2.png"),tr("Sigma Control"), this);
    sigmaCtrl2Action->setShortcut(tr("Ctrl+P"));
    sigmaCtrl2Action->setStatusTip(tr("Open the sigma control window"));
    connect(sigmaCtrl2Action, SIGNAL(triggered()), this, SLOT(OpenSigmaCtrl2Slot()));


    aboutAction = new QAction(tr("About"), this);
    aboutAction->setStatusTip(tr("about this"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(AboutSlot()));
}

void MainWindow::createMenus()
{
    windowMenu = menuBar()->addMenu(tr("Window"));
    windowMenu->addAction(motorizedAction);
    windowMenu->addAction(nanoAction);
    windowMenu->addAction(stimulateAction);
    windowMenu->addAction(fpgaSettingAction);
    windowMenu->addAction(laserCtrlAction);
    windowMenu->addAction(sigmaCtrlAction);
    windowMenu->addAction(sigmaCtrl2Action);


    helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction(aboutAction);


}


void MainWindow::createToolBars()
{
    windowTool = addToolBar("window");
    windowTool->addAction(motorizedAction);
    windowTool->addAction(nanoAction);
    windowTool->addAction(stimulateAction);
    windowTool->addAction(fpgaSettingAction);
    windowTool->addAction(laserCtrlAction);
    windowTool->addAction(sigmaCtrlAction);
    windowTool->addAction(sigmaCtrl2Action);
}





void MainWindow::OpenMotorizedStageSlot()
{
    if(hashChildWindow.contains("motorizedstage"))//子窗口句柄已存在
    {
        qDebug()<<"already open motorizedstage dialog!";
    }
    else
    {
        motorizedstage *motorizedDialog = new motorizedstage(this);
        hashChildWindow.insert("motorizedstage",motorizedDialog);//注册子窗口句柄
        connect(motorizedDialog,&motorizedstage::sendCloseSgn,this,&MainWindow::recieveCloseSlot);//接受子窗口关闭信号，保证一次只有一个子窗口被打开
        connect(motorizedDialog,&motorizedstage::startExposureSgn,this,&MainWindow::motorizedDialogStartExposureSlot);//接收位移台曝光指令
        motorizedDialog->show();
    }
}

void MainWindow::OpenNanoStageSlot()
{
    if(hashChildWindow.contains("nanostage"))//子窗口句柄已存在
    {
        qDebug()<<"already open nanostage dialog!";
    }
    else
    {
        nanostage *nanoDialog = new nanostage(this);
        hashChildWindow.insert("nanostage",nanoDialog);//注册子窗口句柄
        connect(nanoDialog,&nanostage::sendCloseSgn,this,&MainWindow::recieveCloseSlot);//接受子窗口关闭信号，保证一次只有一个子窗口被打开

        nanoDialog->show();
    }
}

void MainWindow::OpenStimulateSlot()
{
    if(hashChildWindow.contains("stimulate"))//子窗口句柄已存在
    {
        qDebug()<<"already open stimulate dialog!";
    }
    else
    {
        stimulate *stimulateDialog = new stimulate(this);
        hashChildWindow.insert("stimulate",stimulateDialog);//注册子窗口句柄
        connect(stimulateDialog,&stimulate::sendCloseSgn,this,&MainWindow::recieveCloseSlot);//接受子窗口关闭信号，保证一次只有一个子窗口被打开
        stimulateDialog->show();
    }
}

void MainWindow::OpenFPGA_SettingSlot()
{
    if(hashChildWindow.contains("fpga_setting"))//子窗口句柄已存在
    {
        qDebug()<<"already open fpga_setting dialog!";
    }
    else
    {
        //FPGA_setting fpgaDialog;// (栈空间由程序自动向操作系统申请分配以及回收),窗口会一闪而过
        FPGA_setting *fpgaDialog = new FPGA_setting(this);//申请堆空间，由程序员申请，由程序员释放，否则则导致内存泄露
        hashChildWindow.insert("fpga_setting",fpgaDialog);//注册子窗口句柄
        connect(fpgaDialog,&FPGA_setting::sendCloseSgn,this,&MainWindow::recieveCloseSlot);//接受子窗口关闭信号，保证一次只有一个子窗口被打开
        connect(this,&MainWindow::sendStartExposureSgn,fpgaDialog,&FPGA_setting::cameraTrigTestSlot);//发送给FPGA界面曝光指令 串口可能未打开，调用就会崩溃！！！
        fpgaDialog->show();
    }
}

void MainWindow::OpenLaserCtrlSlot()
{
    if(hashChildWindow.contains("laserctrl"))//子窗口句柄已存在
    {
        qDebug()<<"already open laserctrl dialog!";
    }
    else
    {
        LaserCtrl *laserctrlDialog=new LaserCtrl(this);
        hashChildWindow.insert("laserctrl",laserctrlDialog);//注册子窗口句柄
        connect(laserctrlDialog,&LaserCtrl::sendCloseSgn,this,&MainWindow::recieveCloseSlot);//接受子窗口关闭信号，保证一次只有一个子窗口被打开
        laserctrlDialog->show();
    }
}


void MainWindow::OpenSigmaCtrlSlot()
{
    if(hashChildWindow.contains(QString("OSM20-85")+QString("TSDM40-15")))//子窗口句柄已存在
    {
        qDebug()<<"already open koki dialog!";
    }
    else
    {
        KokiDialog *kokiDialog = new KokiDialog(this,"OSM20-85","unit is um",1,0,"TSDM40-15","unit is um",0.5,1);
        hashChildWindow.insert(QString("OSM20-85")+QString("TSDM40-15"),kokiDialog);//注册子窗口句柄
        connect(kokiDialog,&KokiDialog::sendCloseSgn,this,&MainWindow::recieveCloseSlot);//接受子窗口关闭信号，保证一次只有一个子窗口被打开
        kokiDialog->setWindowTitle("koki1");
        kokiDialog->show();
    }
}

void MainWindow::OpenSigmaCtrl2Slot()
{
    if(hashChildWindow.contains(QString("OSM20-85")+QString("OSMS_60YAW")))//子窗口句柄已存在
    {
        qDebug()<<"already open koki dialog!";
    }
    else
    {
        KokiDialog *kokiDialog2 = new KokiDialog(this,"OSM20-85","unit is um",1,0,"OSMS_60YAW","unit is degree",0.0025,4);
        hashChildWindow.insert(QString("OSM20-85")+QString("OSMS_60YAW"),kokiDialog2);//注册子窗口句柄
        connect(kokiDialog2,&KokiDialog::sendCloseSgn,this,&MainWindow::recieveCloseSlot);//接受子窗口关闭信号，保证一次只有一个子窗口被打开
        kokiDialog2->setWindowTitle("koki2");
        kokiDialog2->show();
    }
}




void MainWindow::AboutSlot()
{
    qDebug()<<"about";
    //QMessageBox::about(NULL, "About", "Author:  hr_yang\r\n  Email:  hr_yang@pku.edu.cn");
    AboutDialog about;
    about.exec();
}



void MainWindow::motorizedDialogStartExposureSlot()
{
    emit sendStartExposureSgn();
}

void MainWindow::recieveCloseSlot(const QString &str)
{
    if(hashChildWindow.contains(str))
    {
        hashChildWindow.remove(str);
    }
}


void MainWindow::showFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "showFinished: " << exitCode << exitStatus;
}


//    QString programAddress = "C:/Program Files (x86)/SIGMA-KOKI/SGCommander/SGcommander.exe";
//    QStringList arguments;
//    arguments << "";
//    myProcess.start(programAddress,arguments);


