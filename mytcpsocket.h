#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>

//TCP心跳包数据
typedef struct PMC400_STATUS_
{

    qint32 PMC400_X_StartSpeed;
    qint32 PMC400_Y_StartSpeed;
    qint32 PMC400_Z_StartSpeed;
    qint32 PMC400_U_StartSpeed;//初速度

    qint32 PMC400_X_AccSpeed;
    qint32 PMC400_Y_AccSpeed;
    qint32 PMC400_Z_AccSpeed;
    qint32 PMC400_U_AccSpeed;//加速度

    qint32 PMC400_X_FinalSpeed;
    qint32 PMC400_Y_FinalSpeed;
    qint32 PMC400_Z_FinalSpeed;
    qint32 PMC400_U_FinalSpeed;//终速度

    qint32 PMC400_X_CurentPosition;
    qint32 PMC400_Y_CurentPosition;
    qint32 PMC400_Z_CurentPosition;
    qint32 PMC400_U_CurentPosition;//当前位置（输出）

    qint32 PMC400_X_PositionEncode;
    qint32 PMC400_Y_PositionEncode;
    qint32 PMC400_Z_PositionEncode;
    qint32 PMC400_U_PositionEncode;//反馈位置（光栅尺反馈）

    qint16 PMC400_X_Is_MoveCommand;
    qint16 PMC400_Y_Is_MoveCommand;
    qint16 PMC400_Z_Is_MoveCommand;
    qint16 PMC400_U_Is_MoveCommand;//各轴的宏观运动状态： 正向(10)，负向(11)，回0(12)，运动完成(13)，无状态(14)

    qint16 PMC400_IO_IN;//IO输入口（8位）
    qint16 PMC400_IO_OUT;//IO输出口（8位）

    qint16 PMC400_X_Is_Moveing;
    qint16 PMC400_Y_Is_Moveing;
    qint16 PMC400_Z_Is_Moveing;
    qint16 PMC400_U_Is_Moveing;// 各轴是否正在运行，1为停止

    qint32 PMC400_X_Pulse_Remaining;
    qint32 PMC400_Y_Pulse_Remaining;
    qint32 PMC400_Z_Pulse_Remaining;
    qint32 PMC400_U_Pulse_Remaining;// 各轴当前位置：待发出脉冲

    qint16 PMC400_X_MicroSteps;
    qint16 PMC400_Y_MicroSteps;
    qint16 PMC400_Z_MicroSteps;
    qint16 PMC400_U_MicroSteps;// 各轴细分数

    qint32 PMC400_X_ScrewLead;
    qint32 PMC400_Y_ScrewLead;
    qint32 PMC400_Z_ScrewLead;
    qint32 PMC400_U_ScrewLead;// 各轴丝杆导程，单位：nm

    qint32 PMC400_X_SpeedRatio;
    qint32 PMC400_Y_SpeedRatio;
    qint32 PMC400_Z_SpeedRatio;
    qint32 PMC400_U_SpeedRatio;// 各轴转速比，光栅尺走1mm（或1°）需要丝杆转动的圈数, 此处为实际值的1000倍

    qint32 PMC400_X_Electric_Current;
    qint32 PMC400_Y_Electric_Current;
    qint32 PMC400_Z_Electric_Current;
    qint32 PMC400_U_Electric_Current;//各轴电流，暂无用

    qint32 PMC400_X_Resolution_Ratio;
    qint32 PMC400_Y_Resolution_Ratio;
    qint32 PMC400_Z_Resolution_Ratio;
    qint32 PMC400_U_Resolution_Ratio;// 各轴光栅尺分辨率，单位：脉冲数/毫米（脉冲数/度）

    qint32 PMC400_X_Dest_PositionEncode;
    qint32 PMC400_Y_Dest_PositionEncode;
    qint32 PMC400_Z_Dest_PositionEncode;
    qint32 PMC400_U_Dest_PositionEncode;// 各轴绝对位置

    qint32 PMC400_XYZU_EL_ORG; //包含零位,正负限位等状态, x轴: bit0-7; Y轴: bit8-15; Z轴: bit16-23; U轴: bit24-31
    //8bit信息: 4bit-正限位; 5bit-负限位; 6bit-零位;
    qint32 PMC400_PROGRAM_LINE_NUM;// Flash里有具体的运动指令，在程序运动模式下，系统从flash取得指令，一条一条的执行

    qint32 PMC400_X_MoveDistance_Read;
    qint32 PMC400_Y_MoveDistance_Read;
    qint32 PMC400_Z_MoveDistance_Read;
    qint32 PMC400_U_MoveDistance_Read;// 读取到的各轴运动距离在这里


    qint32 PCL400_PROGRAM_LINE_NUM_JP;// 程序运动模式下， 指令的截止数目
    qint32 PMC400_SN; // 序列号


    qint32 PMC400_IP;// IP地址
    qint32 PMC400_NM;// 子网掩码
    qint32 PMC400_GW;// 网关
    qint32 PCL400_32_temp;//备用

    qint32 PCL400_CRC_SUM;// CRC校验, 包括指令的截止数目在内，结构体之前的所有数据的和。
}PMC400_STATUS;




class MyTcpSocket : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);

    ~MyTcpSocket();


    QQueue<QString> tcpQueue;       //定义一个QString型队列，其他常用基本类型都可以定义

    PMC400_STATUS pmc400Status;

    char   isConnect;


private:
    QTcpSocket *socket;



signals:
    void sendMessageBox(const QString &);

public slots:
    void connect();
    void disconnect();

    void socketReadDataSlot();
    void socketDisconnectedSlot();
};

#endif // MYTCPSOCKET_H
