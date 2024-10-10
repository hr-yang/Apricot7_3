#ifndef FPGACHANNEL_H
#define FPGACHANNEL_H

#include <QWidget>
#include <QTableWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QDialog>
#include <QSettings>
#include "table.h"

namespace Ui {
class fpgachannel;
}

class fpgachannel : public QWidget
{
    Q_OBJECT

public:
    explicit fpgachannel(QWidget *parent = nullptr,uchar axis=0);
    ~fpgachannel();

    enum send_mode//与下位机保持一致
    {
        zero=0,
        direct=1,
        min=2,
        max=3,
        double_tri=4,
        single_tri=5,
        step_wave=6,
        square_wave=7
    };

private:
    Ui::fpgachannel *ui;

    uchar channel_axis;

    uchar cur_key_state;
    uchar pre_key_state;

    //自己在头文件中定义的子对象，必须设置对象名称才能存入ini文件。用QT设计师拖入的控件不用，因为在ui头文件里已经设置过对象名称了
    QDialog      *mydialog;
    QSpinBox     *spinBox_SendSelect;
    QPushButton  *pushButton_TableSend;
    Table        *table_DA;

    bool eventFilter(QObject *obj, QEvent *event);//事件过滤器，监测子界面关闭事件
public:
    void setSyn();
public slots:
    void comboxProcessSlot(int);
    void openTableSlot();
    void sendToFPGASlot();
    void sendProcessSlot();
    void setParameters(bool enable, int trigSource, int trigEdge,int trigCount, int waveType, int burstNumber, double period, double phase, double duty, int step, int readLen, double dma_period);
signals:
    void send_DA_Sgn(uchar axis,uchar trig_source,uchar trig_edge,uchar trig_count,
                     uint32_t DA_in, uint32_t DA_min,uint32_t DA_max,
                     uint32_t period,uint32_t duty,uint32_t phase,uint32_t step,uint32_t read_len,
                     uint32_t period_number,uchar mode);

    void send_BRAM_DA_Sgn(uchar axis, uchar table_len,uint16_t *buffer);
    void sendSoftTrigSgn(uchar axis);
};

#endif // FPGACHANNEL_H
