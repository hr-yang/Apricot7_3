#ifndef FPGA_SETTING_H
#define FPGA_SETTING_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QSettings>
#include <QThread>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QLineSeries>
#include <QtCharts/QXYLegendMarker>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QButtonGroup>
#include <QRadioButton>

#include "fpgachannel.h"
#include "fpga_ttl.h"
#include "mychartdata.h"
#include "switchbutton.h"
#include "molisdialog.h"
#include "Global.h"

QT_CHARTS_USE_NAMESPACE


namespace Ui {
class FPGA_setting;
}

class FPGA_setting : public QDialog
{
    Q_OBJECT

public:
    explicit FPGA_setting(QWidget *parent = nullptr);
    ~FPGA_setting();

    void closeEvent(QCloseEvent *event);
private:

    QSerialPort *serial;

    QSettings* settings;
    QSettings* settingsFile0;
    QSettings* settingsFile1;
    QSettings* settingsFile2;
    QSettings* settingsFile3;

    char  isClose;
    int   mRecievedCnt;


    fpgachannel *DA_channel0;
    fpgachannel *DA_channel1;
    FPGA_TTL *TTL_channel0;
    FPGA_TTL *TTL_channel1;
    FPGA_TTL *TTL_channel2;
    FPGA_TTL *TTL_channel3;
    FPGA_TTL *TTL_channel4;
    FPGA_TTL *TTL_channel5;

    //子界面
    QDialog*  gratingDialog;
    QDialog*  simDialog;
    QDialog*  switchDialog;
    MolisDialog*  molisDialog;

    //需要用到的控件
    QLineEdit* mpGratingLineEdit;

    QPushButton* mpComPushButton;
    QComboBox*   mpPortComboBox;

    QButtonGroup* mpProfileButtonGroup;
    QLineEdit *mpExpTimeLineEdit;
    QLabel *mpRecievedCounter;

    QRadioButton* mpProfile1RadioButton;
    QRadioButton* mpProfile2RadioButton;
    QRadioButton* mpProfile3RadioButton;
    QRadioButton* mpProfile4RadioButton;

    //SIM相关控件
    QButtonGroup*  mpTrigEdgeButtonGroup;
    QSpinBox* mpTrigCountSpinBox;
    QSpinBox* mpN_SequenceSpinBox;
    QCheckBox* mpLaser405CheckBox;
    QCheckBox* mpLaser488CheckBox;
    QCheckBox* mpLaser561CheckBox;
    QCheckBox* mpLaser640CheckBox;

    //Switch相关控件
    QButtonGroup* mpSwitch1ButtonGroup;
    QButtonGroup* mpSwitch2ButtonGroup;


    Table*  mpGratingTable;
    QLineEdit* mpGratingInitPos;
    SwitchButton* mpGratingTrigEn;

    //QWidget * chartWidget;
    //MyChartData *mychartdata;
    //QThread     *thread;
    //QChart      *m_chart;
    //QList<QLineSeries *> m_series;

    void DataReceivePrepare(uchar data);
    void DataAnalyze(uchar *data_buf,uint16_t num);

    QString getLastOpenPath();
    void saveLastOpenPath(const QString &path);
public slots:
        void cameraTrigTestSlot();

private slots:     
        void openComButtonSlot();//打开串口
        void readProfileSlot();
        void saveProfileSlot();
        void showChartSlot();
        void showSIMDialogSlot();
        void OnShowSwitchDialog();
        void showGratingDialogSlot();
        void setParameters();
        void synSlot();
        void sendSIMChannelSlot();
        void profileModeButtonsSlot();

        void OnSendGratingTable();
        void OnSendStartGratingTrig();
        void OnSendSwitchData();

        //串口接收槽函数
        void readSerialDataSlot();






        //发送给FPGA的协议
        void send_TTL_Paramters(uchar axis,uchar trig_source,uchar trig_edge,uchar trig_count,
                               uint32_t period,uint32_t duty,uint32_t phase,uint32_t period_number,uchar mode);
        void send_DA_Paramters(uchar axis,uchar trig_source,uchar trig_edge,uchar trig_count,
                                             uint32_t DA_in, uint32_t DA_min,uint32_t DA_max,
                                             uint32_t period,uint32_t duty,uint32_t phase,uint32_t step,uint32_t read_len,
                                             uint32_t period_number,uchar mode);
        void send_BRAM_DA_Data(uchar axis, uchar table_len,uint16_t *buffer);
        void softTrig(uchar order);
        void sendSIMChannel(uchar trig_edge,uchar trig_counter,uchar op_channel,uchar n_sequence);
        void sendSoftReset(void);
        void sendGratingTable(uint32_t trig_sign, uchar trig_number,int32_t *buffer);
        void sendStartGratingTrig(int32_t init_pos,uchar trig_en);
        void sendSwitchData(uchar s1, uchar s2);


        void showMolisDialogSlot();
signals:
        void sendCloseSgn(const QString &);
};

#endif // FPGA_SETTING_H
