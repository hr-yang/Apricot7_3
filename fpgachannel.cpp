#include "fpgachannel.h"
#include "ui_fpgachannel.h"
#include "qt_settings_widgetdatacontrol.h"
#include <QDebug>
#include <QGridLayout>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QtXlsx/QtXlsx>
#include <QStandardPaths>

fpgachannel::fpgachannel(QWidget *parent, uchar axis) :
    QWidget(parent),
    ui(new Ui::fpgachannel)
{
    ui->setupUi(this);

    this->setMinimumSize(QSize( 440, 350 ));

    this->setObjectName("DA_channel"+QString::number(axis));//设置对象名称,为了复制很多份后可以互相区分，参数才可以各自保存！！！

    ui->groupBox->setTitle("DA_channel "+QString::number(axis));

    channel_axis=axis;

    //setnow button组设置
    ui->buttonGroup->addButton(ui->radioButton_Pos,0);
    ui->buttonGroup->addButton(ui->radioButton_Neg,1);
    ui->radioButton_Pos->setChecked(true);  //设置初选项

    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboxProcessSlot(int)));

    //页面部件设置,防止界面显示错乱
    comboxProcessSlot(ui->comboBox->currentIndex());
    ui->radioButton_Pos->setChecked(true);


    //OpenTable UI子界面
    //自己在头文件中定义的子对象，必须设置对象名称才能存入ini文件。用QT设计师拖入的控件不用，因为在ui头文件里已经设置过对象名称了
    mydialog  = new QDialog(this);
    mydialog->setObjectName("mydialog");//必须设置对象名称才能存入文件，否则为空
    spinBox_SendSelect=new QSpinBox(this);
    spinBox_SendSelect->setObjectName("sendSelect");
    pushButton_TableSend =new QPushButton(mydialog);
    pushButton_TableSend->setObjectName("pushButton_TableSend");
    table_DA =new Table(this);
    QLabel *lable=new QLabel(this);

    mydialog->setWindowTitle("CH"+QString::number(channel_axis));
    mydialog->installEventFilter(this);//安装事件过滤器，监视此部件的关闭事件
    lable->setText("SelectColumnNo:");
    spinBox_SendSelect->setMinimum(1);//实际编号是从0开始
    pushButton_TableSend->setText("SendToFPGA");
    pushButton_TableSend->setDefault(false);
    pushButton_TableSend->setAutoDefault(false);//不能让键盘聚焦在此按钮

    //布局
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hlayout->addWidget(lable);
    hlayout->addWidget(spinBox_SendSelect);
    hlayout->addWidget(pushButton_TableSend);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(table_DA);
    layout->addItem(new QSpacerItem(20, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->addLayout(hlayout);
    mydialog->setLayout(layout);

    connect(ui->pushButton_SetValue,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_Plus,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_Minus,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_Max,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_Min,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_Mid,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_SetMax,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_SetMin,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_SetMid,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->pushButton_Send,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);
    connect(ui->PushButton_SoftTrig,&QPushButton::clicked,this,&fpgachannel::sendProcessSlot);

    connect(ui->pushButton_OpenTable,&QPushButton::clicked,this,&fpgachannel::openTableSlot);
    connect(pushButton_TableSend,&QPushButton::clicked,this,&fpgachannel::sendToFPGASlot);
}


fpgachannel::~fpgachannel()
{
    delete ui;
}

//事件过滤器
bool fpgachannel::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==mydialog)
    {
        if(event->type()==QEvent::Close)//Dialog关闭事件
        {
             ui->pushButton_OpenTable->setText(tr("OpenTable"));
             return true;
        }
        else
            return false;
    }
    else
        return fpgachannel::eventFilter(obj,event);
}


void fpgachannel::comboxProcessSlot(int id)
{
    //页面设置
    if(id<4)
    {
        ui->stackedWidget_DA->setCurrentIndex(0);//AXI DA页面
        ui->stackedWidget_Mode->setCurrentIndex(id);    //每个波形的专属子页面
    }
    else if(id==4)
        ui->stackedWidget_DA->setCurrentIndex(1);//BRAM DA页面
    else if(id==5)
        ui->stackedWidget_DA->setCurrentIndex(2);//DMA DA页面
}




//BRAM DA页面中的按钮
void fpgachannel::openTableSlot()
{
    if(ui->pushButton_OpenTable->text() == tr("OpenTable"))
    {
        ui->pushButton_OpenTable->setText(tr("CloseTable"));
        mydialog->show();
    }
    else
    {
        ui->pushButton_OpenTable->setText(tr("OpenTable"));
        mydialog->close();
    }
}








static uint16_t tableBuffer[200];

void fpgachannel::sendToFPGASlot()
{
    //判断需要发送哪一列
    int sendColumnNo=spinBox_SendSelect->value();
    if(sendColumnNo>table_DA->p_table->columnCount())
    {
        QMessageBox::information(nullptr, "Table", "ColumnNumberError!", QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    sendColumnNo=sendColumnNo-1;//实际表格是从0开始编号
    for(int i=0; i<table_DA->p_table->rowCount(); i++)//行必须从0开始
    {
            if(table_DA->p_table->item(i, sendColumnNo)!=nullptr)//如果TableWidget中内容为空的时候访问数据,则会造成卡死！所以先判断文本是否为空.
            {
                QString  temp = table_DA->p_table->item(i,sendColumnNo)->text();
                float value=temp.toFloat();
                value = value>= -5? (value<=5? value:5) : -5;//value 在-5~5
                uint16_t value_temp=(value+5)/10.0f*16383 + 0.5f;
                tableBuffer[i]=value_temp;
            }
            else
                tableBuffer[i]=0;
    }
    uchar table_len = table_DA->p_table->rowCount();
    qDebug()<<table_len;
    emit send_BRAM_DA_Sgn(channel_axis,table_len,tableBuffer);
}

//发送处理
void fpgachannel::sendProcessSlot()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if(btn->objectName()=="pushButton_Send")//Send按钮按下
    {
        //触发源参数
        uchar trig_source = ui->comboBox_Source->currentIndex();
        uchar trig_edge   = ui->buttonGroup->checkedId();
        uchar trig_count  = ui->spinBox_Count->value();

        //波形幅值参数
        float   max_value =QString(ui->lineEdit_Max->text()).toFloat();
        float   min_value =QString(ui->lineEdit_Min->text()).toFloat();
        max_value = max_value>= -5? (max_value<=5? max_value:5) : -5;//max_value 在-5~5
        min_value = min_value>= -5? (min_value<=5? min_value:5) : -5;//min_value 在-5~5
        int max_value_tmp=(max_value+5)/10.0f*16383 + 0.5f;
        int min_value_tmp=(min_value+5)/10.0f*16383 + 0.5f;


        //波形时间参数
        uint32_t   period_number = ui->lineEdit_PeriodNumber->text().toInt();
        float      period        = QString(ui->lineEdit_Period->text()).toFloat();
        float      phase         = QString(ui->lineEdit_Phrase->text()).toFloat();
        float      duty          = QString(ui->lineEdit_duty->text()).toFloat();
        int        step          = QString(ui->stepLineEdit->text()).toInt();
        int        read_len      = QString(ui->lineEdit_ReadLen->text()).toInt();
        int        start_value   = (max_value_tmp-min_value_tmp)*phase/period+min_value_tmp;//计算波形起始电压
        uchar      mode          = ui->comboBox->currentIndex()+4;//  加4是波形模式偏置，前面分别是零值、当前值、最小值、最大值

        //所有参数都必须转化为整形
        int period_tmp = 50000000 * period/1000.0;   //周期数
        int phase_tmp  = 50000000 * phase /1000.0; //相位数
        int duty_tmp   = 50000000 * duty  /1000.0; //占空比数

        if(mode==9)//DMA DA
        {
            float period  = QString(ui->lineEdit_Period_DMADA->text()).toFloat();
            period_tmp=   50000000 * period/1000.0;   //周期数
        }

        //发送信号
        emit send_DA_Sgn(channel_axis ,trig_source,trig_edge,trig_count,
                         start_value, min_value_tmp,max_value_tmp,  //传错变量类型,float传入uint32_t,导致数据错误 大坑！！！
                         period_tmp,duty_tmp,phase_tmp,step,read_len,
                         period_number,mode);
    }
    else if(btn->objectName()=="PushButton_SoftTrig")//软触发按钮按下
    {
        emit sendSoftTrigSgn(channel_axis+6);//这里把uaxis编号转化为order    前6个轴是TTL通道
    }
    else//调整和设置波形参数值
    {
        float step_value;
        float value=0;
        enum send_mode mode;

        if(btn->objectName()=="pushButton_SetValue")
        {
            value=QString(ui->lineEdit_Value->text()).toFloat();
        }
        else if(btn->objectName()=="pushButton_Plus")
        {
            value=QString(ui->lineEdit_Value->text()).toFloat();
            step_value=QString(ui->lineEdit_Step->text()).toFloat();
            value+=step_value;
            ui->lineEdit_Value->setText(QString::number(value));
        }
        else if(btn->objectName()=="pushButton_Minus")
        {
            value=QString(ui->lineEdit_Value->text()).toFloat();
            step_value=QString(ui->lineEdit_Step->text()).toFloat();
            value-=step_value;
            ui->lineEdit_Value->setText(QString::number(value));
        }

        else if(btn->objectName()=="pushButton_SetMin")//设置最小值
        {
            value=QString(ui->lineEdit_Value->text()).toFloat();
            ui->lineEdit_Min->setText(QString::number(value));


            if(cur_key_state==1)//如果上一时刻按下的还是此按键
                pre_key_state=pre_key_state;
            else
                pre_key_state=cur_key_state;

            if(pre_key_state==3)//如果先前设置的是最大值
            {
                float maxvalue=ui->lineEdit_Max->text().toFloat();
                ui->lineEdit_Mid->setText(QString::number((value+maxvalue)/2));

            }
            else if(pre_key_state==2)//如果先前设置的是中间值
            {
                float midvalue=ui->lineEdit_Mid->text().toFloat();
                ui->lineEdit_Max->setText(QString::number(2*midvalue-value));
            }

            cur_key_state=1;
        }
        else if(btn->objectName()=="pushButton_SetMid")//设置中间值
        {
            value=QString(ui->lineEdit_Value->text()).toFloat();
            ui->lineEdit_Mid->setText(QString::number(value));

            if(cur_key_state==2)//如果上一时刻按下的还是此按键
                pre_key_state=pre_key_state;
            else
                pre_key_state=cur_key_state;

            if(pre_key_state==3)//如果先前设置的是最大值
            {
                float maxvalue=ui->lineEdit_Max->text().toFloat();
                ui->lineEdit_Min->setText(QString::number(2*value-maxvalue));
            }
            else if(pre_key_state==1)//如果先前设置的是最小值
            {
                float minvalue=ui->lineEdit_Min->text().toFloat();
                ui->lineEdit_Max->setText(QString::number(2*value-minvalue));
            }

            cur_key_state=2;
        }
        else if(btn->objectName()=="pushButton_SetMax")//设置最大值
        {

            float minvalue=ui->lineEdit_Min->text().toFloat();
            value=QString(ui->lineEdit_Value->text()).toFloat();
            if(value<=minvalue)   return;   //最大值必须大于最小值

            ui->lineEdit_Max->setText(QString::number(value));

            if(cur_key_state==3)//如果上一时刻按下的还是此按键，之前按键状态不变
                pre_key_state=pre_key_state;
            else
                pre_key_state=cur_key_state;


            if(pre_key_state==1)//如果先前设置的是最小值
            {
                ui->lineEdit_Mid->setText(QString::number((value+minvalue)/2));
            }
            else if(pre_key_state==2)//如果先前设置的是中间值
            {
                float midvalue=ui->lineEdit_Mid->text().toFloat();
                ui->lineEdit_Min->setText(QString::number(2*midvalue-value));
            }
            cur_key_state=3;
        }
        else if(btn->objectName()=="pushButton_Max")
        {
            ui->lineEdit_Value->setText(ui->lineEdit_Max->text());
            value=QString(ui->lineEdit_Value->text()).toFloat();
        }
        else if(btn->objectName()=="pushButton_Min")
        {
            ui->lineEdit_Value->setText(ui->lineEdit_Min->text());
            value=QString(ui->lineEdit_Value->text()).toFloat();
        }
        else if(btn->objectName()=="pushButton_Mid")
        {
            ui->lineEdit_Value->setText(ui->lineEdit_Mid->text());
            value=QString(ui->lineEdit_Value->text()).toFloat();
        }

        value = value>= -5? (value<=5? value:5) : -5;//value 在-5~5
        int value_tmp=(value+5)/10.0f*16383 + 0.5f;
        mode=direct;
        emit send_DA_Sgn(channel_axis ,0,0,1,
                         value_tmp, 0,0,
                         0,0,0,0,0,
                         0,mode);
    }
}




void fpgachannel::setSyn()//一键设置同步功能
{
    ui->pushButton_Send->click();//点击发送按钮
}


