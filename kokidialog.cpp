#include "kokidialog.h"
#include "ui_kokidialog.h"
#include "qt_settings_widgetdatacontrol.h"

#include <QDebug>
#include <QSettings>
#include <QCloseEvent>
#include <QMessageBox>
#include <QStandardPaths>


//GSC-02控制器

KokiDialog::KokiDialog(QWidget *parent,QString stage1,QString label1,float resolu1,int precision1,
                                       QString stage2,QString label2,float resolu2,int precision2) :
    QDialog(parent),
    ui(new Ui::KokiDialog)
{
    ui->setupUi(this);

    this->setFixedSize(610,585);

    dialogName=stage1+stage2;

    ui->groupBox_Stage1->setTitle(stage1);
    ui->groupBox_Stage2->setTitle(stage2);

    ui->label_Stage1->setText(label1);
    ui->label_Stage2->setText(label2);

    resolution=resolu1;
    resolution_2=resolu2;

    precis1=precision1;
    precis2=precision2;


    this->setObjectName(dialogName);//防止配置文件窜扰


    //radio button组设置
    ui->buttonGroup_FH->addButton(ui->radioButton_Free,0);
    ui->buttonGroup_FH->addButton(ui->radioButton_Hold,1);

    ui->radioButton_Hold->setChecked(true);  // 为组设置初选项


    ui->buttonGroup_FH_2->addButton(ui->radioButton_Free_2,0);
    ui->buttonGroup_FH_2->addButton(ui->radioButton_Hold_2,1);

    ui->radioButton_Hold_2->setChecked(true);  // 为组设置初选项

    //查找可用的串口
    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox_Port->addItem(serial.portName());
            serial.close();
        }
    }
    //设置波特率下拉菜单默认显示第0项
    ui->comboBox_Port->setCurrentIndex(0);

    isClose=1;//串口关闭标志

    //创建和读取一样  必须在save之后磁盘才会有该文件
    settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/ApricotConfig.ini", QSettings::IniFormat,this);
    settings->setIniCodec("UTF-8");

    Qt_Settings_WidgetDataControl::read(settings,this, Qt_Settings_WidgetDataControl::controlFlgasWidgets);

    //设置部件初始状态
    ui->comboBox_Port->setEnabled(true);
    ui->pushButton_OpenCom->setText(tr("OpenCOM"));
    ui->radioButton_Hold->setChecked(true);  // 为组设置初选项
    ui->radioButton_Hold_2->setChecked(true);// 为组设置初选项

    A=ui->lineEdit_A->text().toFloat()/resolution;
    B=ui->lineEdit_B->text().toFloat()/resolution;
    C=ui->lineEdit_C->text().toFloat()/resolution;
    A_2=ui->lineEdit_A_2->text().toFloat()/resolution_2;
    B_2=ui->lineEdit_B_2->text().toFloat()/resolution_2;
    C_2=ui->lineEdit_C_2->text().toFloat()/resolution_2;
    stage_dir1=ui->comboBox->currentText();
    stage_dir2=ui->comboBox_2->currentText();


    ui->pushButton_A->setEnabled(false);
    ui->pushButton_A_2->setEnabled(false);
    ui->pushButton_B->setEnabled(false);
    ui->pushButton_B_2->setEnabled(false);
    ui->pushButton_C->setEnabled(false);
    ui->pushButton_C_2->setEnabled(false);
    ui->pushButton_Minus->setEnabled(false);
    ui->pushButton_Minus_2->setEnabled(false);
    ui->pushButton_Plus->setEnabled(false);
    ui->pushButton_Plus_2->setEnabled(false);

    setAttribute(Qt::WA_DeleteOnClose);//因为当关闭子窗体时不会调用析构函数，只有关闭父窗体时，才会调用子窗体的析构函数，解决办法加入这句语句
}

KokiDialog::~KokiDialog()
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



void KokiDialog::closeEvent(QCloseEvent *event)
{
    emit sendCloseSgn(dialogName);//发送注销子窗口句柄信号
    event->accept();
}



void KokiDialog::on_pushButton_OpenCom_clicked()
{
    if(ui->pushButton_OpenCom->text() == tr("OpenCOM"))
        {
            serial = new QSerialPort;
            //设置串口名
            serial->setPortName(ui->comboBox_Port->currentText());
            //打开串口
            serial->open(QIODevice::ReadWrite);
            //设置波特率
            serial->setBaudRate(QSerialPort::Baud9600);
            //设置数据位数
            serial->setDataBits(QSerialPort::Data8);//设置数据位8
            //设置校验位
            serial->setParity(QSerialPort::NoParity);
            //设置停止位
            serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
            //设置流控制
            serial->setFlowControl(QSerialPort::HardwareControl);//设置为硬件流控制
            //关闭设置按钮
            ui->comboBox_Port->setEnabled(false);
            ui->pushButton_OpenCom->setText(tr("CloseCOM"));

            //只有打开了串口才连接这些跟串口有关的信号槽,防止串口未打开的时候按下程序崩溃!!!
            connect(serial,&QSerialPort::readyRead,this,&KokiDialog::readSerialDataSlot);

            isClose=0;//串口打开标志
        }
        else
        {
            //关闭串口
            serial->clear();
            serial->close();
            serial->deleteLater();

            isClose=1;//串口关闭标志

            //恢复按钮
            ui->comboBox_Port->setEnabled(true);
            ui->pushButton_OpenCom->setText(tr("OpenCOM"));

            //串口关闭后断开这些跟串口有关的信号槽,防止串口未打开的时候按下程序崩溃!!!
            disconnect(serial,&QSerialPort::readyRead,this,&KokiDialog::readSerialDataSlot);
        }
}


void KokiDialog::readSerialDataSlot()//串口接受槽函数
{
    QByteArray temp;
    temp = serial->readAll();
    if(!temp.isEmpty())
    {
        if(flag==1)
        {
            if(temp!="\n")//帧尾不对，接收错误
            {
                flag=0;
                buffer.clear();
            }
            else//接收完成一帧
            {
                buffer.append(temp);
                flag=0;
                finished_flag=1;
            }
        }
        else//可能temp大于一个字符
        {
            if(temp.contains("\r\n"))//接收到\r\n    一帧结尾
            {
                buffer.append(temp);
                finished_flag=1;
            }
            else if(temp.contains("\r"))//接收到\r
            {
                buffer.append(temp);
                flag=1;
            }
            else
            {
                buffer.append(temp);
            }
        }
        temp.clear();
    }


    if(finished_flag==1)
    {
        finished_flag=0;

        ui->lineEdit_CurPos->setText(QString::number(QString(buffer.left(10)).remove(QRegExp("\\s")).toInt()*resolution,10,precis1));//Returns a byte array that contains the leftmost 10 bytes of this byte array
        ui->lineEdit_CurPos_2->setText(QString::number(QString(buffer.mid(11,10)).remove(QRegExp("\\s")).toInt()*resolution_2,10,precis2));//Returns a byte array containing 10 bytes from this byte array, starting at position 11

        if((*(buffer.end()-3))=='B')//忙则继续查询    因为使用迭代器后解引用的是字符类型，不是字符串类型
        {
            serial->write("Q:\r\n");
            serial->flush();
        }
        else if((*(buffer.end()-3))=='R')//就绪
        {
            ready=1;
        }
        buffer.clear();
    }
}





//Stage1


void KokiDialog::on_pushButton_Home_clicked()
{
    if(isClose==1)  return;

    if(stage_dir1=="+") serial->write("H:1-\r\n");
    else serial->write("H:1+\r\n");
    serial->flush();
    serial->write("Q:\r\n");
    serial->flush();

    ui->pushButton_A->setEnabled(true);
    ui->pushButton_B->setEnabled(true);
    ui->pushButton_C->setEnabled(true);
    ui->pushButton_Minus->setEnabled(true);
    ui->pushButton_Plus->setEnabled(true);
}


void KokiDialog::on_pushButton_SetSpeed_clicked()
{
    if(isClose==1)  return;

    range=ui->lineEdit_Range->text().toInt();
    slow=(int)(ui->lineEdit_Slow->text().toFloat()/resolution);
    fast=(int)(ui->lineEdit_Fast->text().toFloat()/resolution);
    rate=ui->lineEdit_Rate->text().toInt();
    slow_2=(int)(ui->lineEdit_Slow_2->text().toFloat()/resolution_2);
    fast_2=(int)(ui->lineEdit_Fast_2->text().toFloat()/resolution_2);
    rate_2=ui->lineEdit_Rate_2->text().toInt();

    QString temp="D:"+QString::number(range)+"S"+QString::number(slow)+"F"+QString::number(fast)+"R"+QString::number(rate)
                +"S"+QString::number(slow_2)+"F"+QString::number(fast_2)+"R"+QString::number(rate_2)+"\r\n";

    serial->write(temp.toLatin1().data());
    serial->flush();
}

void KokiDialog::on_radioButton_Free_clicked()
{
    if(isClose==1)  return;

    serial->write("C:10\r\n");
    serial->flush();
}

void KokiDialog::on_radioButton_Hold_clicked()
{
    if(isClose==1)  return;

    serial->write("C:11\r\n");
    serial->flush();
}

void KokiDialog::on_pushButton_Stop_clicked()
{
    if(isClose==1)  return;

    serial->write("L:E\r\n");
    serial->flush();
}

void KokiDialog::on_pushButton_Plus_clicked()
{
    if(isClose==1)  return;

    QString pulse=QString::number((int)(ui->lineEdit_Amount->text().toFloat()/resolution));
    QString temp="M:1+P"+pulse+"\r\n";
    serial->write(temp.toLatin1().data());
    serial->flush();
    serial->write("G\r\n");
    serial->flush();
    serial->write("Q:\r\n");
    serial->flush();
}

void KokiDialog::on_pushButton_Minus_clicked()
{
    if(isClose==1)  return;

    QString pulse=QString::number((int)(ui->lineEdit_Amount->text().toFloat()/resolution));
    QString temp="M:1-P"+pulse+"\r\n";
    serial->write(temp.toLatin1().data());
    serial->flush();
    serial->write("G\r\n");
    serial->flush();
    serial->write("Q:\r\n");
    serial->flush();
}

void KokiDialog::on_pushButton_SetA_clicked()
{
    if(isClose==1)  return;

    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    A=(int)(ui->lineEdit_CurPos->text().toFloat()/resolution);
    ui->lineEdit_A->setText(QString::number(A*resolution));
}

void KokiDialog::on_pushButton_SetB_clicked()
{
    if(isClose==1)  return;

    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int temp_B=(int)(ui->lineEdit_CurPos->text().toFloat()/resolution);

    if(stage_dir1=="+"?temp_B>A:temp_B<A)
    {
       B=temp_B;
       ui->lineEdit_B->setText(QString::number(B*resolution));
    }
    else
       QMessageBox::warning(this,"warning","|B| must > |A|",QMessageBox::Yes);
}

void KokiDialog::on_pushButton_SetC_clicked()
{
    if(isClose==1)  return;

    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int temp_C=(int)(ui->lineEdit_CurPos->text().toFloat()/resolution);

    if(stage_dir1=="+"?temp_C>B:temp_C<B)
    {
            C=temp_C;
            ui->lineEdit_C->setText(QString::number(C*resolution));
    }
    else
            QMessageBox::warning(this,"warning","|C| must > |B|",QMessageBox::Yes);

}


void KokiDialog::on_pushButton_A_clicked()
{
    if(isClose==1)  return;

    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int pos_now=(int)(ui->lineEdit_CurPos->text().toFloat()/resolution);

    if(stage_dir1=="+"?pos_now<A:pos_now>A)
    {
        QString temp="M:1"+stage_dir1+"P"+QString::number(A-pos_now)+"\r\n";//+
        serial->write(temp.toLatin1().data());
        serial->flush();
        serial->write("G\r\n");
        serial->flush();
        serial->write("Q:\r\n");
        serial->flush();
    }
    else if(pos_now==A)
    {


    }
    else
    {
        QMessageBox::StandardButton button;
        button=QMessageBox::question(this,tr("反向"),QString(tr("是否回机械零点？")),QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::Cancel)
        {

        }
        else if(button==QMessageBox::No)
        {

        }
        else if(button==QMessageBox::Yes)
        {
            on_pushButton_Home_clicked();
        }
    }
}

void KokiDialog::on_pushButton_B_clicked()
{
    if(isClose==1)  return;

    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int pos_now=(int)(ui->lineEdit_CurPos->text().toFloat()/resolution);
    if(stage_dir1=="+"?pos_now<B:pos_now>B)
    {
        QString temp="M:1"+stage_dir1+"P"+QString::number(B-pos_now)+"\r\n";//+
        serial->write(temp.toLatin1().data());
        serial->flush();
        serial->write("G\r\n");
        serial->flush();
        serial->write("Q:\r\n");
        serial->flush();
    }
    else if(pos_now==B)
    {


    }
    else
    {
        QMessageBox::StandardButton button;
        button=QMessageBox::question(this,tr("反向"),QString(tr("是否回机械零点？")),QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::Cancel)
        {

        }
        else if(button==QMessageBox::No)
        {

        }
        else if(button==QMessageBox::Yes)
        {
            on_pushButton_Home_clicked();
        }
    }
}

void KokiDialog::on_pushButton_C_clicked()
{
    if(isClose==1)  return;


    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int pos_now=(int)(ui->lineEdit_CurPos->text().toFloat()/resolution);
    if(stage_dir1=="+"?pos_now<C:pos_now>C)
    {
        QString temp="M:1"+stage_dir1+"P"+QString::number(C-pos_now)+"\r\n";//+
        serial->write(temp.toLatin1().data());
        serial->flush();
        serial->write("G\r\n");
        serial->flush();
        serial->write("Q:\r\n");
        serial->flush();
    }
    else if(pos_now==C)
    {


    }
    else
    {
        QMessageBox::StandardButton button;
        button=QMessageBox::question(this,tr("反向"),QString(tr("是否回机械零点？")),QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::Cancel)
        {

        }
        else if(button==QMessageBox::No)
        {

        }
        else if(button==QMessageBox::Yes)
        {
            on_pushButton_Home_clicked();
        }
    }
}


//Stage2


void KokiDialog::on_pushButton_Home_2_clicked()
{
    if(isClose==1)  return;

    if(stage_dir2=="+") serial->write("H:2-\r\n");
    else serial->write("H:2+\r\n");
    serial->flush();
    serial->write("Q:\r\n");
    serial->flush();

    ui->pushButton_A_2->setEnabled(true);
    ui->pushButton_B_2->setEnabled(true);
    ui->pushButton_C_2->setEnabled(true);
    ui->pushButton_Minus_2->setEnabled(true);
    ui->pushButton_Plus_2->setEnabled(true);
}


void KokiDialog::on_pushButton_SetSpeed_2_clicked()
{
    if(isClose==1)  return;

    range=ui->lineEdit_Range->text().toInt();
    slow=(int)(ui->lineEdit_Slow->text().toFloat()/resolution);
    fast=(int)(ui->lineEdit_Fast->text().toFloat()/resolution);
    rate=ui->lineEdit_Rate->text().toInt();
    slow_2=(int)(ui->lineEdit_Slow_2->text().toFloat()/resolution_2);
    fast_2=(int)(ui->lineEdit_Fast_2->text().toFloat()/resolution_2);
    rate_2=ui->lineEdit_Rate_2->text().toInt();

    QString temp="D:"+QString::number(range)+"S"+QString::number(slow)+"F"+QString::number(fast)+"R"+QString::number(rate)
                +"S"+QString::number(slow_2)+"F"+QString::number(fast_2)+"R"+QString::number(rate_2)+"\r\n";

    serial->write(temp.toLatin1().data());
    serial->flush();
}

void KokiDialog::on_radioButton_Free_2_clicked()
{
    if(isClose==1)  return;

    serial->write("C:20\r\n");
    serial->flush();
}

void KokiDialog::on_radioButton_Hold_2_clicked()
{
    if(isClose==1)  return;

    serial->write("C:21\r\n");
    serial->flush();
}

void KokiDialog::on_pushButton_Stop_2_clicked()
{
    if(isClose==1)  return;

    serial->write("L:E\r\n");
    serial->flush();
}

void KokiDialog::on_pushButton_Plus_2_clicked()
{
    if(isClose==1)  return;

    QString pulse=QString::number((int)(ui->lineEdit_Amount_2->text().toFloat()/resolution_2));
    QString temp="M:2+P"+pulse+"\r\n";
    serial->write(temp.toLatin1().data());
    serial->flush();
    serial->write("G\r\n");
    serial->flush();
    serial->write("Q:\r\n");
    serial->flush();
}

void KokiDialog::on_pushButton_Minus_2_clicked()
{
    if(isClose==1)  return;

    QString pulse=QString::number((int)(ui->lineEdit_Amount_2->text().toFloat()/resolution_2));
    QString temp="M:2-P"+pulse+"\r\n";
    serial->write(temp.toLatin1().data());
    serial->flush();
    serial->write("G\r\n");
    serial->flush();
    serial->write("Q:\r\n");
    serial->flush();
}

void KokiDialog::on_pushButton_SetA_2_clicked()
{
    if(isClose==1)  return;
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    A_2=(int)(ui->lineEdit_CurPos_2->text().toFloat()/resolution_2);
    ui->lineEdit_A_2->setText(QString::number(A_2*resolution_2));
}

void KokiDialog::on_pushButton_SetB_2_clicked()
{
    if(isClose==1)  return;
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int temp_B=(int)(ui->lineEdit_CurPos_2->text().toFloat()/resolution_2);
    if(stage_dir2=="+"?temp_B>A_2:temp_B<A_2)
    {
        B_2=temp_B;
        ui->lineEdit_B_2->setText(QString::number(B_2*resolution_2));
    }
    else
        QMessageBox::warning(this,"warning","|B| must > |A|",QMessageBox::Yes);
}

void KokiDialog::on_pushButton_SetC_2_clicked()
{
    if(isClose==1)  return;
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int temp_C=(int)(ui->lineEdit_CurPos_2->text().toFloat()/resolution_2);
    if(stage_dir2=="+"?temp_C>B_2:temp_C<B_2)
    {
        C_2=temp_C;
        ui->lineEdit_C_2->setText(QString::number(C_2*resolution_2));
    }
    else
        QMessageBox::warning(this,"warning","|C| must > |B|",QMessageBox::Yes);
}


void KokiDialog::on_pushButton_A_2_clicked()
{
    if(isClose==1)  return;

    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int pos_now=(int)(ui->lineEdit_CurPos_2->text().toFloat()/resolution_2);
    if(stage_dir2=="+"?pos_now<A_2:pos_now>A_2)
    {
        QString temp="M:2"+stage_dir2+"P"+QString::number(A_2-pos_now)+"\r\n";//-
        serial->write(temp.toLatin1().data());
        serial->flush();
        serial->write("G\r\n");
        serial->flush();
        serial->write("Q:\r\n");
        serial->flush();
    }
    else if(pos_now==A_2)
    {
    }
    else
    {
        QMessageBox::StandardButton button;
        button=QMessageBox::question(this,tr("反向"),QString(tr("是否回机械零点？")),QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::Cancel)
        {

        }
        else if(button==QMessageBox::No)
        {

        }
        else if(button==QMessageBox::Yes)
        {
            on_pushButton_Home_2_clicked();
        }
    }
}

void KokiDialog::on_pushButton_B_2_clicked()
{
    if(isClose==1)  return;

    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int pos_now=(int)(ui->lineEdit_CurPos_2->text().toFloat()/resolution_2);
    if(stage_dir2=="+"?pos_now<B_2:pos_now>B_2)
    {
        QString temp="M:2"+stage_dir2+"P"+QString::number(B_2-pos_now)+"\r\n";
        serial->write(temp.toLatin1().data());
        serial->flush();
        serial->write("G\r\n");
        serial->flush();
        serial->write("Q:\r\n");
        serial->flush();
    }
    else if(pos_now==B_2)
    {
    }
    else
    {
        QMessageBox::StandardButton button;
        button=QMessageBox::question(this,tr("反向"),QString(tr("是否回机械零点？")),QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::Cancel)
        {

        }
        else if(button==QMessageBox::No)
        {

        }
        else if(button==QMessageBox::Yes)
        {
            on_pushButton_Home_2_clicked();
        }
    }
}

void KokiDialog::on_pushButton_C_2_clicked()
{
    if(isClose==1)  return;

    //更新位置
    serial->write("Q:\r\n");
    serial->flush();
    while(ready!=1);//等待不忙
    ready=0;

    int pos_now=(int)(ui->lineEdit_CurPos_2->text().toFloat()/resolution_2);
    if(stage_dir2=="+"?pos_now<C_2:pos_now>C_2)
    {
        QString temp="M:2"+stage_dir2+"P"+QString::number(C_2-pos_now)+"\r\n";
        serial->write(temp.toLatin1().data());
        serial->flush();
        serial->write("G\r\n");
        serial->flush();
        serial->write("Q:\r\n");
        serial->flush();
    }
    else if(pos_now==C_2)
    {
    }
    else
    {
        QMessageBox::StandardButton button;
        button=QMessageBox::question(this,tr("反向"),QString(tr("是否回机械零点？")),QMessageBox::Cancel|QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::Cancel)
        {

        }
        else if(button==QMessageBox::No)
        {

        }
        else if(button==QMessageBox::Yes)
        {
            on_pushButton_Home_2_clicked();
        }
    }
}
