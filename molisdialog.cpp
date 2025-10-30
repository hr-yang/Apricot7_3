#include "molisdialog.h"
#include "ui_molisdialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QtXlsx/QtXlsx>
#include <QStandardPaths>

MolisDialog::MolisDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MolisDialog)
{
    ui->setupUi(this);
}

MolisDialog::~MolisDialog()
{
    delete ui;
}

void MolisDialog::on_pushButtonCalc_clicked()
{
    double  Z0  =  ui->doubleSpinBoxZ0->value(); //光片照明起始位置
    double H0   = ui->doubleSpinBoxH0->value(); //光片厚度
    double L    = ui->doubleSpinBoxL->value(); //一次光场成像中，光片之间的距离
    int     M   = ui->spinBoxM->value(); //调制次数
    int     K   = ui->spinBoxK->value(); //每张光场中光片的照明数量

    int nColumnCount = M;    //获取列数
    int nRowCount    = K;       //获取行数

    //先把table的内容清空
    ui->tableWidget->clear();
    for (int n=0; n<ui->tableWidget->columnCount(); n++)
    {
        ui->tableWidget->removeColumn(0);
    }

    //根据Excel设置表尺寸
    ui->tableWidget->setColumnCount(nColumnCount);
    ui->tableWidget->setRowCount(nRowCount);

    double value=Z0;
    //插入数据
    for(int i=0;i<nRowCount;i++)//行列都从0开始，否则会卡死！！！
    {
        for(int j=0; j<nColumnCount; j++)
        {
            ui->tableWidget->setItem(i,j, new QTableWidgetItem(QString::number(value)));
            value=value+H0;
        }
    }

    ui->tableWidget->resizeColumnsToContents();
}

void MolisDialog::on_pushButtonExport_clicked()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, tr("Save Excel"),
                                            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), tr("Excel File(*.xlsx)"));
    if (fileName.isEmpty())
    {
        return;
    }

    QXlsx::Document xlsx(fileName);
    int tableR = ui->tableWidget->rowCount();
    int tableC = ui->tableWidget->columnCount();

    double pos0um=ui->doubleSpinBoxPos0um->value();
    double pos100um=ui->doubleSpinBoxPos100um->value();

    //写数据
    for (int j=0; j<tableC; j++)
    {
        for (int i=0; i<tableR; i++)
        {
            if ( ui->tableWidget->item(i,j) != nullptr )
            {
                double value=ui->tableWidget->item(i,j)->text().toDouble();
                xlsx.write(j*tableR+i+1, 1,value);//注意xlsx文件的起始行列都从1开始
                xlsx.write(j*tableR+i+1, 2,value*(pos100um-pos0um)/100+pos0um);//注意xlsx文件的起始行列都从1开始
            }
        }
    }
    xlsx.saveAs(fileName);//写完之后一定要保存
}

void MolisDialog::on_pushButton_Update_clicked()
{
    if(ui->tabWidget_Mode->currentIndex()==0) //MoLiS模式
    {
        qDebug()<<"MoLiS Mode";
        double DA0_T=ui->doubleSpinBox_DA0_T->value();
        double D=ui->doubleSpinBox_D->value();
        double R=ui->doubleSpinBox_R->value();
        int I=ui->spinBox_I->value();
        int N=ui->spinBox_N->value();

        double  Z0  =  ui->doubleSpinBoxZ0->value(); //光片照明起始位置
        double  H0  = ui->doubleSpinBoxH0->value(); //光片厚度
        double  L   = ui->doubleSpinBoxL->value(); //一次光场成像中，光片之间的距离
        int     M   = ui->spinBoxM->value(); //调制次数
        int     K   = ui->spinBoxK->value(); //每张光场中光片的照明数量

    //    1. Galvo快轴调制信号（DA0）
    //       波形：正弦波（DMA_DA)
    //       周期T：1ms
    //       触发源： 软触发（SYN）
    //       触发边沿：POS
    //       触发计数：1
        emit setDAParameters(0, true, SOFT, POS, 1, DMA_DA, 0, 0, 0, 0, 0, 0, DA0_T);

    //    2. Laser调制信号（TTL1）
    //       正脉宽W=快轴Galvo周期DA0_T
    //       周期T=正脉宽+Galvo运动响应时间D
    //       触发源： 外部触发EXT0（相机全局曝光信号）
    //       触发边沿：Pos
    //       触发计数：1
    //       猝发数目：每张光场图片中光片照明数量K
         double TTL1_T=DA0_T+D;
         emit setTTLParameters(1,true, EXT0, POS, 1, K, TTL1_T, 0, DA0_T);

    //    3. Galvo慢轴调制信号（DA1）
    //       波形：BRAM_DA（需要设置波表）
    //       触发源： Larser调制信号（TTL1）
    //       触发边沿：Neg
    //       触发计数：1
          emit setDAParameters(1, true, TTL1, NEG, 1, BRAM_DA, 0, 0, 0, 0, 0, M*K, 0);

    //    相机全局曝光信号时间E=Laser调制信号周期TTL1_T x K - Galvo运动响应时间D
          double E=TTL1_T*K-D;

    //    4. 相机触发信号（TTL0）
    //       正脉宽W=1ms
    //       周期T=相机全局曝光信号时间E+相机读出时间R
    //       触发源： 体周期信号（TTL4）
    //       触发边沿：Pos
    //       触发计数：1
    //       猝发数目：调制次数M + 1（相机使用同步触发模式）
          double TTL0_T=E+R;
          emit setTTLParameters(0,true, TTL4, POS, 1, M+1, TTL0_T, 0, 1);

    //    单个体成像时间V=相机触发信号周期(TTL0_T) x 调制次数M
          double V=TTL0_T*M;

    //    5. 相机去尾信号（TTL5）
    //       正脉宽=单个体成像时间V+相机读出时间R/2（确保信号与运算无毛刺）
    //       周期=尽可能设置大些（大于TTL4周期）
    //       触发源： 体周期信号（TTL4）
    //       触发边沿：Neg
    //       触发计数：1
    //       猝发数目：1
          double TTL5_W=V+R/2;
          double TTL4_T=TTL5_W+I;
          double TTL5_T=TTL4_T+10;
          emit setTTLParameters(5,true, TTL4, NEG, 1, 1, TTL5_T, 0, TTL5_W);

    //    6. 体周期信号（TTL4）
    //       正脉宽=2ms
    //       周期=相机去尾信号正脉宽TTL5_W+体间隔时间I
    //       触发源： 软触发（SYN）
    //       触发边沿：Pos
    //       触发计数：1
    //       猝发数目：体成像数目
          emit setTTLParameters(4,true, SYN, POS, 1, N, TTL5_W+I, 0, 2);

          emit setTTLParameters(2,false, SYN, POS, 1,   0, 2, 0, 1);
          emit setTTLParameters(3,false, SYN, POS, 1,   0, 2, 0, 1);
    }
    else if(ui->tabWidget_Mode->currentIndex()==1)  //光片模式
    {

        qDebug()<<"LS Mode";

        double CameraExposure=ui->doubleSpinBox_CameraExposure->value(); //相机曝光时间
        double PiezoResponse=ui->doubleSpinBox_PiezoResponse->value(); //Piezo运动响应时间
        int LayersNum=ui->spinBox_LayerNum->value();//光片照明层数

        /*
         * 1. Piezo触发信号 (TTL2)
         *      正脉宽:PiezoResponse
         *      周期=PiezoResponse+CameraExposure+0.2
         *      触发源： 软触发（SYN）
         *      触发边沿：Pos
         *      触发计数：1
         *      猝发数目：LayersNum+1
         */
         emit setTTLParameters(2,true, SYN, POS, 1, LayersNum+1, PiezoResponse+CameraExposure+0.2, 0, PiezoResponse);

        /*
         * 2. 相机触发信号 (TTL0)
         *      正脉宽:CameraExposure
         *      周期=CameraExposure+0.2
         *      触发源： TTL2
         *      触发边沿：Neg
         *      触发计数：1
         *      猝发数目：1
         */
         emit setTTLParameters(0,true, TTL2, NEG, 1, 1, CameraExposure+0.2, 0, CameraExposure);

        /*
         * 3. 激光器信号 (TTL1)
         *      正脉宽: CameraExposure
         *      周期=CameraExposure+0.2
         *      触发源： TTL2
         *      触发边沿：Neg
         *      触发计数：1
         *      猝发数目：1
         */
         emit setTTLParameters(1,true, TTL2, NEG, 1, 1, CameraExposure+0.2, 0, CameraExposure);

        /* 4. Galvo慢轴调制信号（DA1）
         *      波形：STEP
         *      触发源： 软触发（SYN）
         *      触发边沿：Pos
         *      触发计数：1
         *      周期：PiezoResponse+CameraExposure+0.2
         * */
         emit setDAParameters(1, true,  SYN, POS, 1,    STEP_WAVE, 0,   PiezoResponse+CameraExposure+0.2, 0, 0,    LayersNum,  0, 0);


        /* 5. Galvo快轴调制信号（DA0）
         *       波形：正弦波（DMA_DA)
         *       周期T：1ms
         *       触发源： 软触发（SYN）
         *       触发边沿：POS
         *       触发计数：1
         * */
         emit setDAParameters(0, true, SOFT, POS, 1, DMA_DA, 0, 0, 0, 0, 0, 0, 1);


        emit setTTLParameters(3,false, SYN, POS, 1,   0, 2, 0, 1);
        emit setTTLParameters(4,false, SYN, POS, 1,   0, 2, 0, 1);
        emit setTTLParameters(5,false, SYN, POS, 1,   0, 2, 0, 1);

    // void setTTLParameters(int ch, bool enable, int trigSource, int trigEdge, int trigCount, int burstNumber, double period, double phase, double duty);
    // void setDAParameters(int ch, bool enable, TrigSource trigSource, TrigEdge trigEdge,int trigCount, WaveType waveType, int burstNumber, double period, double phase, double duty, int step, int readLen, double dma_period);
    }
    else if(ui->tabWidget_Mode->currentIndex()==2)  //单平面光场模式
    {

        qDebug()<<"SP Mode";

        double CameraExposure=ui->doubleSpinBox_CameraExposure_SP->value(); //相机曝光时间
        double PiezoResponse=ui->doubleSpinBox_PiezoResponse_SP->value(); //Piezo运动响应时间
        int LayersNum=ui->spinBox_LayerNum_SP->value();//光片照明层数

        /*
         * 1. Piezo触发信号 (TTL2)
         *      正脉宽:PiezoResponse
         *      周期=PiezoResponse+CameraExposure+0.2
         *      触发源： 软触发（SYN）
         *      触发边沿：Pos
         *      触发计数：1
         *      猝发数目：LayersNum+1
         */
         emit setTTLParameters(2,true, SYN, POS, 1, LayersNum+1, PiezoResponse+CameraExposure+0.2, 0, PiezoResponse);

        /*
         * 2. 相机触发信号 (TTL0)
         *      正脉宽:CameraExposure
         *      周期=CameraExposure+0.2
         *      触发源： TTL2
         *      触发边沿：Neg
         *      触发计数：1
         *      猝发数目：1
         */
         emit setTTLParameters(0,true, TTL2, NEG, 1, 1, CameraExposure+0.2, 0, CameraExposure);

        /*
         * 3. 激光器信号 (TTL1)
         *      正脉宽: CameraExposure
         *      周期=CameraExposure+0.2
         *      触发源： TTL2
         *      触发边沿：Neg
         *      触发计数：1
         *      猝发数目：1
         */
         emit setTTLParameters(1,true, TTL2, NEG, 1, 1, CameraExposure+0.2, 0, CameraExposure);

        /* 3. Galvo慢轴调制信号（DA1）
         *      波形：DIRECT
         *      触发源： 软触发（SYN）
         *      触发边沿：Pos
         *      触发计数：1
         *      周期：PiezoResponse+CameraExposure+0.2
         * */
         emit setDAParameters(1, true,  SYN, POS, 1,    DIRECT, 0,   0, 0, 0,    0,  0, 0);

        /* 4. Galvo快轴调制信号（DA0）
         *       波形：正弦波（DMA_DA)
         *       周期T：1ms
         *       触发源： 软触发（SYN）
         *       触发边沿：POS
         *       触发计数：1
         * */
         emit setDAParameters(0, true, SOFT, POS, 1, DMA_DA, 0, 0, 0, 0, 0, 0, 1);

         emit setTTLParameters(3,false, SYN, POS, 1,   0, 2, 0, 1);
         emit setTTLParameters(4,false, SYN, POS, 1,   0, 2, 0, 1);
         emit setTTLParameters(5,false, SYN, POS, 1,   0, 2, 0, 1);

    // void setTTLParameters(int ch, bool enable, int trigSource, int trigEdge, int trigCount, int burstNumber, double period, double phase, double duty);
    // void setDAParameters(int ch, bool enable, TrigSource trigSource, TrigEdge trigEdge,int trigCount, WaveType waveType, int burstNumber, double period, double phase, double duty, int step, int readLen, double dma_period);
    }
    else if(ui->tabWidget_Mode->currentIndex()==3)  //光场模式
    {

        qDebug()<<"LF Mode";

        int Num=ui->spinBox_LFNum->value();//单次拍摄体数目
        double Interval=ui->doubleSpinBox_LFInterval->value();//间隔时间
        int Cycle=ui->spinBox_LFCycle->value();//重复次数
        double CameraReadout=ui->doubleSpinBox_LFCameraReadout->value(); //相机曝光时间
        double CameraExposure=ui->doubleSpinBox_LFCameraExposure->value(); //相机曝光时间

        double TTL0_T=CameraExposure+CameraReadout;
        /*
         * 1. 相机触发信号 (TTL0)
         *      正脉宽:CameraExposure
         *      周期=CameraExposure+CameraReadout
         *      触发源： TTL4
         *      触发边沿：Pos
         *      触发计数：1
         *      猝发数目：Num
         */
         emit setTTLParameters(0,true,
                               TTL4, POS, 1,  //触发设置
                               Num, TTL0_T, 0, CameraExposure);//猝发、周期、相位、正脉宽

        /*
         * 2. 激光器信号 (TTL1)
         *      正脉宽: CameraExposure
         *      周期=CameraExposure+CameraReadout
         *      触发源： TTL0
         *      触发边沿：Pos
         *      触发计数：1
         *      猝发数目：1
         */
         emit setTTLParameters(1,true,
                               TTL0, POS, 1, //触发设置
                               1, TTL0_T, 0, CameraExposure);//猝发、周期、相位、正脉宽

        /*
         *   3. 体周期信号（TTL4）
         *       正脉宽=0.2ms
         *       周期=相机触发信号周期TTL0_T x Num + 体间隔时间Interval
         *       触发源： 软触发（SYN）
         *       触发边沿：Pos
         *       触发计数：1
         *       猝发数目：循环次数Cycle
         * */
         emit setTTLParameters(4,true,
                               SYN, POS, 1, //触发设置
                               Cycle, TTL0_T*Num+Interval, 0, 0.2);//猝发、周期、相位、正脉宽

        /* 4. Galvo慢轴调制信号（DA1）
         *      波形：SingleTriangle
         *      触发源： TTL0
         *      触发边沿：Pos
         *      触发计数：1
         *      周期：CameraExposure
         * */
         emit setDAParameters(1, true,
                              TTL0, POS, 1,   //触发设置
                              SINGLE_TRIANGLE,  //波形类型
                              1,  CameraExposure, 0, 0,  //猝发、周期、相位、正脉宽
                              0, 0, 0); //STEP、ReadLen、DMA_Period

        /* 5. Galvo快轴调制信号（DA0）
         *       波形：正弦波（DMA_DA)
         *       周期T：1ms
         *       触发源： 软触发（SYN）
         *       触发边沿：POS
         *       触发计数：1
         * */
         emit setDAParameters(0, true,
                              SOFT, POS, 1, //触发设置
                              DMA_DA, //波形类型
                              0, 0, 0, 0, //猝发、周期、相位、正脉宽
                              0, 0, 1); //STEP、ReadLen、DMA_Period

         emit setTTLParameters(2,false, SYN, POS, 1,   0, 2, 0, 1);
         emit setTTLParameters(3,false, SYN, POS, 1,   0, 2, 0, 1);
         emit setTTLParameters(5,false, SYN, POS, 1,   0, 2, 0, 1);
    }
}
