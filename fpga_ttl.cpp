#include "fpga_ttl.h"
#include "ui_fpga_ttl.h"
#include <QDebug>


FPGA_TTL::FPGA_TTL(QWidget *parent, uchar axis) :
    QWidget(parent),
    ui(new Ui::FPGA_TTL)
{
    ui->setupUi(this);

    this->setMinimumSize( QSize( 450, 290 ));

    this->setObjectName("TTL_channel"+QString::number(axis));//设置对象名称,为了复制很多份后可以互相区分，参数才可以各自保存！！！

    channel_axis=axis;

    ui->groupBox->setTitle("TTL_channel "+QString::number(axis));

    //setnow button组设置
    ui->setNowButtonGroup->addButton(ui->radioButton_Pos,0);
    ui->setNowButtonGroup->addButton(ui->radioButton_Neg,1);
    ui->radioButton_Pos->setChecked(true);  //设置初选项

    //多个按钮连接同一个槽
    connect(ui->pushButton_High,&QPushButton::clicked,this,&FPGA_TTL::sendProcessSlot);
    connect(ui->pushButton_Low,&QPushButton::clicked,this,&FPGA_TTL::sendProcessSlot);
    connect(ui->pushButton_Send,&QPushButton::clicked,this,&FPGA_TTL::sendProcessSlot);
    connect(ui->pushButton_SoftTrig,&QPushButton::clicked,this,&FPGA_TTL::sendProcessSlot);
}

FPGA_TTL::~FPGA_TTL()
{
    delete ui;
}


void FPGA_TTL::sendProcessSlot()
{

    uchar mode;

    QPushButton* btn = qobject_cast<QPushButton*>(sender());

    if(btn->objectName()=="pushButton_High")
    {
        mode=MAX;
        emit send_TTL_ParamtersSgn(channel_axis,0,0,1,
                              0,0,0,0,mode);
    }
    else if(btn->objectName()=="pushButton_Low")
    {
        mode=MIN;
        emit send_TTL_ParamtersSgn(channel_axis,0,0,1,
                              0,0,0,0,mode);
    }
    else if(btn->objectName()=="pushButton_Send")
    {
        uchar trig_source = ui->comboBox_Source->currentIndex();
        uchar trig_edge   = ui->setNowButtonGroup->checkedId();
        uchar trig_count  = ui->spinBox_Count->value();

        float period = QString(ui->lineEdit_Period->text()).toFloat();
        float phase= QString(ui->lineEdit_Phase->text()).toFloat();
        float duty = QString(ui->lineEdit_Duty->text()).toFloat();
        int period_tmp = 50000000 * period/1000.0;   //周期数
        int phase_tmp  = 50000000 * phase /1000.0; //相位数
        int duty_tmp   = 50000000 * duty  /1000.0; //占空比数

        mode=SQUARE_WAVE;
        uint32_t period_number=ui->lineEdit_PeriodNumber->text().toInt();

        emit send_TTL_ParamtersSgn(channel_axis,trig_source,trig_edge,trig_count,
                                   period_tmp,duty_tmp,phase_tmp,period_number,mode);
    }
    else if(btn->objectName()=="pushButton_SoftTrig")
    {
        emit softTrig(channel_axis);
    }
}

void FPGA_TTL::setParameters(bool enable, TrigSource trigSource, TrigEdge trigEdge,int trigCount, int burstNumber, double period, double phase, double duty)
{
    ui->groupBox->setChecked(enable);

    ui->comboBox_Source->setCurrentIndex(trigSource);
    ui->setNowButtonGroup->button(trigEdge)->setChecked(true);
    ui->spinBox_Count->setValue(trigCount);

    ui->lineEdit_PeriodNumber->setText(QString::number(burstNumber));
    ui->lineEdit_Period->setText(QString::number(period));
    ui->lineEdit_Phase->setText(QString::number(phase));
    ui->lineEdit_Duty->setText(QString::number(duty));
}

void FPGA_TTL::sendParameters()//一键设置同步功能
{
   ui->pushButton_Send->click();//点击发送按钮
}

