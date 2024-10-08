#ifndef MYCHARTDATA_H
#define MYCHARTDATA_H

#include <QObject>
#include <QPointF>


class MyChartData : public QObject
{
    Q_OBJECT
public:
    explicit MyChartData(QObject *parent = nullptr);

    QList<QPointF> data;

    static bool cmp(QPointF a, QPointF b); //定义为静态，不然出错


    enum send_mode//与下位机保持一致
    {
        zero=0,
        syn_gun=1,
        min=2,
        max=3,
        direct=4,
        double_tri=5,
        single_tri=6,
        square_wave=7,
        step_wave=8,
        duty_sin_tri=9,
    };



    int maxvalue;
    int minvalue;



signals:
    void addseries();
public slots:
    void chartDataProcessSlot(uchar channel_axis, uchar mode, uchar set_now, uchar period_number, int step, float value, float freq, float phase, int duty);

};

#endif // MYCHARTDATA_H
