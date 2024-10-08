#include "mychartdata.h"
#include <QDebug>
#include <QThread>

MyChartData::MyChartData(QObject *parent) : QObject(parent)
{

}

bool MyChartData::cmp(QPointF a, QPointF b)
{
    return a.x()>b.x();
}



void MyChartData::chartDataProcessSlot(uchar channel_axis, uchar mode, uchar set_now, uchar period_number, int step, float value, float freq, float phase, int duty)
{
    /**************************************************传输协议***********************************************************/
    //字节号       0            1              2             3         4~7           8~11            12~15      16~19
    //        axis mode     set_now     period_number      step       period(freq)   phase          duty      NA
    /*******************************************************************************************************************/

    Q_UNUSED(channel_axis);
    Q_UNUSED(set_now);
    Q_UNUSED(period_number);
    qDebug()<<"mychartdata thread id: "<<QThread::currentThreadId();

    if(mode==1)
    {

    }
    else if(mode==2) //模式2-设置DA最小值
    {
        float vol = value>= -5? (value<=5? value:5) : -5;//vol 在-5~5
        int VOL_TMP = (vol+5)/10.0f*16383 + 0.5f;
        minvalue=VOL_TMP;
    }
    else if(mode==3)//模式3-设置DA最大值
    {
            float vol = value>= -5? (value<=5? value:5) : -5;//vol 在-5~5
            int VOL_TMP = (vol+5)/10.0f*16383 + 0.5f;

            maxvalue=VOL_TMP;
    }
    else if(mode ==4)
    {

    }
    else//波形模式
    {
            int period;
            int phase_;
            int duty_;

            int clock=freq*360;//50000000;

            period = clock/freq+0.5f;//周期变量
            if(mode==step_wave)//step波形模式
            {
                 period = clock/freq/step+0.5f;//每一小步的周期数
                 step=0;
            }

            int periodtemp = clock/freq+0.5f;//临时周期变量

            phase_ = periodtemp * phase/360.0+0.5f;//相位  0~360

            if(mode == square_wave||mode==duty_sin_tri)//方波或具有占空比的单边三角波
            {
                duty_ =periodtemp* duty/100.0+0.5f;
            }
            else
            {
                duty_ = 0;
            }

            int    period_counter;//周期计数变量(period_counter)
            int    ref_counter;//参考的计数变量

            period_counter = phase_;//从多少个数开始，等价于相位
            ref_counter =0; //参考计数变量，从0开始，为了保证波形整体相移后周期的完整性

            int V_delta=(maxvalue-minvalue)/period;//步进量
            int DA_BuffA=minvalue;
            int DA_BuffB=maxvalue;

            int V_Tmp = 0;
            int step_counter = 0;//阶梯波的阶梯计数变量
            int period_half=period/2;

            qDebug()<<"max:"<<maxvalue;
            qDebug()<<"min:"<<minvalue;
            qDebug()<<"step:"<<step;
            qDebug()<<"period:"<<period;
            qDebug()<<"phase:"<<phase_;
            qDebug()<<"duty:"<<duty_;

            data.clear();

            while(1)
            {
                switch(mode)
                {
                    case 5://mode5 双边三角波

                                    if(period_counter>0 && period_counter<period_half)   //双边三角波
                                        V_Tmp =  V_Tmp + V_delta;
                                    else if(period_counter > period_half && period_counter < period)
                                        V_Tmp =  V_Tmp - V_delta;
                                    else if(period_counter == 0)
                                        V_Tmp = DA_BuffA;
                                    else if(period_counter == period_half)
                                        V_Tmp = DA_BuffB;
                                    else if(period_counter == period)
                                        V_Tmp = DA_BuffA;
                    break;

                    case 6://mode6 单边三角波

                                     if(period_counter>0 && period_counter<period)  //单边三角波
                                            V_Tmp =  V_Tmp + V_delta;
                                     else if(period_counter == 0)
                                            V_Tmp = DA_BuffA;
                                     else if(period_counter == period)
                                            V_Tmp = DA_BuffB;

                    break;

                    case 7://mode7  方波

                                    if(period_counter>0 &&  period_counter< duty)
                                        V_Tmp = DA_BuffA;
                                    else
                                        V_Tmp = DA_BuffB;

                    break;

                    case 8://mode8 阶梯波
                         if(period_counter == period)//每到了一个周期开始执行加delta
                         {
                                 if(step_counter < step-1)//为了包含第0步,就少算一步
                                 {
                                       step_counter = step_counter+1;
                                       V_Tmp =  V_Tmp + V_delta;
                                 }
                                 else
                                 {
                                       step_counter = 0;
                                       V_Tmp = DA_BuffA;
                                 }
                         }
                    break;

                    case 9://mode9  具有占空比的单边三角波
                          if(period_counter>0 && period_counter<duty_-1)  //单边三角波
                                 V_Tmp =  V_Tmp + V_delta;
                          else if(period_counter == duty_ - 1)
                                 V_Tmp = DA_BuffB;
                          else
                                 V_Tmp = DA_BuffA;
                    break;
                }

                //对period_counter周期变量进行处理
                if(period_counter < period)    //0，1，2，...,period
                          period_counter= period_counter + 1;
                else
                          period_counter = 0;

                if(ref_counter < period)
                         ref_counter = ref_counter + 1;
                else//只画一个周期
                {
                         break;
                }
                data.append(QPointF(ref_counter, V_Tmp));
                qDebug()<<"data:"<<data.last();
          }

          //qSort(data.begin(),data.end(),cmp);
          emit addseries();
    }


    qDebug()<<"exit mychartdata";
}
















