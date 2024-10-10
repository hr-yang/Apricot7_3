#ifndef FPGA_TTL_H
#define FPGA_TTL_H

#include <QWidget>

namespace Ui {
class FPGA_TTL;
}

class FPGA_TTL : public QWidget
{
    Q_OBJECT

public:
    explicit FPGA_TTL(QWidget *parent = 0,uchar axis=0);
    ~FPGA_TTL();

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
    Ui::FPGA_TTL *ui;

    uchar channel_axis;//该通道编号

public:
    void setSyn();//一键设置同步功能,并使通道回到初始状态

public slots:
    void sendProcessSlot();
    void setParameters(bool enable, int trigSource, int trigEdge, int trigCount, int burstNumber, double period, double phase, double duty);

signals:
    void send_TTL_ParamtersSgn(uchar axis,uchar trig_source,uchar trig_edge,uchar trig_count,
                               uint32_t period,uint32_t duty,uint32_t phase,uint32_t period_number,uchar mode);
    void sendSoftTrigSgn(uchar axis);
};

#endif // FPGA_TTL_H

