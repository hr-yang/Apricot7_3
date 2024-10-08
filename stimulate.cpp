#include "stimulate.h"
#include "ui_stimulate.h"
#include "qt_settings_widgetdatacontrol.h"

#include <QDebug>
#include <QSettings>
#include <QCloseEvent>
#include <QStandardPaths>



stimulate::stimulate(QWidget *parent) :
    QDialog(parent),
    serial(nullptr),
    settings(nullptr),
    ui(new Ui::stimulate)
{
    ui->setupUi(this);

    this->setFixedSize(310,470);

    connect(ui->openComButton,&QPushButton::clicked,this,&stimulate::openComButtonSlot);//打开串口按钮


    //radioButton组设置
    ui->ledModeButtonGroup->addButton(ui->ledMode1,0);
    ui->ledModeButtonGroup->addButton(ui->ledMode2,1);  

    ui->ledMode1->setChecked(true);  // 为组设置初选项

    connect(ui->ledMode1, &QRadioButton::clicked, this, &stimulate::ledModeButtonsClicked);
    connect(ui->ledMode2, &QRadioButton::clicked, this, &stimulate::ledModeButtonsClicked);


    //led方向 button组设置
    ui->buttonGroupDirection->addButton(ui->radioButtonDirLeft,0);
    ui->buttonGroupDirection->addButton(ui->radioButtonDirRight,1);

    ui->radioButtonDirLeft->setChecked(true);  // 为组设置初选项

    ui->lightStimulateStackedWidget->setCurrentIndex(0);


    //connect(ui->singleTime,SIGNAL(valueChanged(int)),this,SLOT(singleTimeSlot(int)));//只要键盘按下键就会触发槽，输入两位数以上会连续触发槽，导致下位机串口接收丢帧
    //ui->singleTime->installEventFilter(this);//为这个spinbox安装事件过滤器，监视此部件

    //查找可用的串口
    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->portBox->addItem(serial.portName());
            serial.close();
        }
    }
    //设置波特率下拉菜单默认显示第0项
    ui->baudBox->setCurrentIndex(0);


    //创建和读取一样  必须在save之后磁盘才会有该文件
    settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/ApricotConfig.ini", QSettings::IniFormat,this);
    settings->setIniCodec("UTF-8");

    Qt_Settings_WidgetDataControl::read(settings,this, Qt_Settings_WidgetDataControl::controlFlgasWidgets);


    //按钮组多页面部件设置
    ui->lightStimulateStackedWidget->setCurrentIndex(ui->ledModeButtonGroup->checkedId());
    //恢复设置使能
    ui->portBox->setEnabled(true);
    ui->baudBox->setEnabled(true);
    ui->openComButton->setText(tr("OpenCOM"));



    isClose=1;//串口关闭标志



    setAttribute(Qt::WA_DeleteOnClose);//因为当关闭子窗体时不会调用析构函数，只有关闭父窗体时，才会调用子窗体的析构函数，解决办法加入这句语句

}

stimulate::~stimulate()
{

    Qt_Settings_WidgetDataControl::save(settings,this, Qt_Settings_WidgetDataControl::controlFlgasWidgets);

    if(isClose==0)//串口未关闭
    {
        serial->clear();
        serial->close();
        serial->deleteLater();
    }


    delete ui;
}

void stimulate::closeEvent(QCloseEvent *event)
{
    emit sendCloseSgn("stimulate");//发送注销子窗口句柄信号
    event->accept();
}


//事件过滤器
//bool mainwindow::eventFilter(QObject *obj, QEvent *event)
//{
//	if(obj==ui->singleTime)
//	{
//		if(event->type()==QEvent::KeyPress)
//		{
//			QKeyEvent *keyEvent=static_cast<QKeyEvent *>(event);
//			if(keyEvent->key()==Qt::Key_Return)
//			{
//				qDebug()<<"singleTimeKeyEvent:"<<ui->singleTime->value();
//				QString str="mode1"+QString::number(ui->singleTime->value())+"\r\n";
//				qDebug()<<str;
//				char*  p;
//				QByteArray ba = str.toLatin1(); // must
//				p=ba.data();
//				serial->write(p);
//
//				return true;
//			}
//			else
//				return false;
//		}
//		else
//			return false;
//	}
//	else
//		return QMainWindow::eventFilter(obj,event);
//}


void stimulate::openComButtonSlot()//打开串口
{
    if(ui->openComButton->text() == tr("OpenCOM"))
    {
        serial = new QSerialPort;
        //设置串口名
        serial->setPortName(ui->portBox->currentText());
        //打开串口
        serial->open(QIODevice::ReadWrite);
        //设置波特率
        serial->setBaudRate(ui->baudBox->currentText().toInt());
        //设置数据位数
        serial->setDataBits(QSerialPort::Data8);//设置数据位8
        //设置校验位
        serial->setParity(QSerialPort::NoParity);
        //设置停止位
        serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
        //设置流控制
        serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

        //关闭设置菜单使能
        ui->portBox->setEnabled(false);
        ui->baudBox->setEnabled(false);
        ui->openComButton->setText(tr("CloseCOM"));

        //只有打开了串口才连接这些跟串口有关的信号槽,防止串口未打开的时候按下程序崩溃!!!
        connect(serial,&QSerialPort::readyRead,this,&stimulate::readSerialDataSlot);

        connect(ui->setMode1Button,&QPushButton::clicked,this,&stimulate::setMode1ButtonSlot);//mode1发送参数按钮
        connect(ui->brightnessSlider,&QSlider::sliderReleased,this,&stimulate::brightSliderReleaseSlot);//mode2  亮度设置
        connect(ui->resetButton,&QPushButton::clicked,this,&stimulate::resetButtonSlot);//发送复位信息
        connect(ui->pushButton_RunOnce,&QPushButton::clicked,this,&stimulate::runOnceSlot);

        isClose=0;//串口打开标志
    }
    else
    {
        //关闭串口
        serial->clear();
        serial->close();
        serial->deleteLater();

        isClose=1;//串口关闭标志

        ////恢复设置使能
        ui->portBox->setEnabled(true);
        ui->baudBox->setEnabled(true);
        ui->openComButton->setText(tr("OpenCOM"));

        //串口关闭后断开这些跟串口有关的信号槽,防止串口未打开的时候按下程序崩溃!!!
        disconnect(serial,&QSerialPort::readyRead,this,&stimulate::readSerialDataSlot);

        disconnect(ui->setMode1Button,&QPushButton::clicked,this,&stimulate::setMode1ButtonSlot);//mode1发送参数按钮
        disconnect(ui->brightnessSlider,&QSlider::sliderReleased,this,&stimulate::brightSliderReleaseSlot);//mode2  亮度设置
        disconnect(ui->resetButton,&QPushButton::clicked,this,&stimulate::resetButtonSlot);//发送复位信息
        disconnect(ui->pushButton_RunOnce,&QPushButton::clicked,this,&stimulate::runOnceSlot);

    }
}

void stimulate::readSerialDataSlot()//串口接受槽函数
{
    QByteArray buf;
    buf = serial->readAll();
    if(!buf.isEmpty())
    {
        //QString str = ui->serialTextEdit->toPlainText();
       // str+=tr(buf);
       // ui->serialTextEdit->clear();
       // ui->serialTextEdit->append(str);
    }
    buf.clear();
}


void stimulate::ledModeButtonsClicked()//选择不同的led模式显示不同的子界面
{
    switch(ui->ledModeButtonGroup->checkedId())
    {
    case 0:
        qDebug() << "mode 0";
        ui->lightStimulateStackedWidget->setCurrentIndex(0);
        break;
    case 1:
        qDebug() << "mode 1";
        ui->lightStimulateStackedWidget->setCurrentIndex(1);
        break; 
    }
}


void stimulate::setMode1ButtonSlot()
{


    qDebug()<<"singleTime:"<<ui->singleTime->value();
    qDebug()<<"repetTime:"<<ui->repetTime->value();


    //mode1+0/1+singleTime+and+rapeatTime    0:left 1:right
    QString str="mode1"+QString::number(ui->buttonGroupDirection->checkedId())+QString::number(ui->singleTime->value())+"and"+QString::number(ui->repetTime->value())+"\r\n";
    qDebug()<<str;
    char*  p;
    QByteArray ba = str.toLatin1(); // must
    p=ba.data();
    serial->write(p);

}


void stimulate::brightSliderReleaseSlot()//led亮度模式下的亮度值改变
{
    QString value=QString::number(ui->brightnessSlider->value());
    ui->brightnessLabel->setText(value+"%");
    QString str="mode2"+value+"\r\n";
    char*  p;
    QByteArray ba = str.toLatin1(); // must
    p=ba.data();
    serial->write(p);

}


void stimulate::resetButtonSlot()
{
    serial->write("mode3\r\n");
}

void stimulate::runOnceSlot()
{

    qDebug()<<"singleTime:"<<ui->singleTime->value();

    //mode4+0/1+singleTime   0:left 1:right
    QString str="mode4"+QString::number(ui->buttonGroupDirection->checkedId())+QString::number(ui->singleTime->value())+"\r\n";
    qDebug()<<str;
    char*  p;
    QByteArray ba = str.toLatin1(); // must
    p=ba.data();
    serial->write(p);
}
