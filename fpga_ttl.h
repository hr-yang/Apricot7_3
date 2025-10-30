#ifndef FPGA_TTL_H
#define FPGA_TTL_H

#include <QWidget>
#include "Global.h"

namespace Ui {
class FPGA_TTL;
}

class FPGA_TTL : public QWidget
{
    Q_OBJECT

public:
    explicit FPGA_TTL(QWidget *parent = 0,uchar axis=0);
    ~FPGA_TTL();

private:
    Ui::FPGA_TTL *ui;

    uchar channel_axis;//该通道编号

public:
    void sendParameters();//一键设置同步功能,并使通道回到初始状态

public slots:
    void sendProcessSlot();
    void setParameters(bool enable, TrigSource trigSource, TrigEdge trigEdge, int trigCount, int burstNumber, double period, double phase, double duty);

signals:
    void send_TTL_ParamtersSgn(uchar axis,uchar trig_source,uchar trig_edge,uchar trig_count,
                               uint32_t period,uint32_t duty,uint32_t phase,uint32_t period_number,uchar mode);
    void softTrig(uchar axis);
};






#endif // FPGA_TTL_H

