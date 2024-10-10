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
    double DA0_T=ui->doubleSpinBox_DA0_T->value();
    double D=ui->doubleSpinBox_D->value();
    double R=ui->doubleSpinBox_R->value();
    int I=ui->spinBox_I->value();
    int N=ui->spinBox_N->value();

    double  Z0  =  ui->doubleSpinBoxZ0->value(); //光片照明起始位置
    double H0   = ui->doubleSpinBoxH0->value(); //光片厚度
    double L    = ui->doubleSpinBoxL->value(); //一次光场成像中，光片之间的距离
    int     M   = ui->spinBoxM->value(); //调制次数
    int     K   = ui->spinBoxK->value(); //每张光场中光片的照明数量

//    1. Galvo快轴调制信号（DA0）
//       波形：正弦波（DMA_DA)
//       周期T：1ms
//       触发源： 软触发（SYN）
//       触发边沿：POS
//       触发计数：1
    emit setDAParameters(0, true, 0, 0, 1, 5, 0, 0, 0, 0, 0, 0, DA0_T);

//    2. Laser调制信号（TTL1）
//       正脉宽W=快轴Galvo周期DA0_T
//       周期T=正脉宽+Galvo运动响应时间D
//       触发源： 外部触发EXT0（相机全局曝光信号）
//       触发边沿：Pos
//       触发计数：1
//       猝发数目：每张光场图片中光片照明数量K
     double TTL1_T=DA0_T+D;
     emit setTTLParameters(1,true, 2, 0, 1, K, TTL1_T, 0, DA0_T);

//    3. Galvo慢轴调制信号（DA1）
//       波形：BRAM_DA（需要设置波表）
//       触发源： Larser调制信号（TTL1）
//       触发边沿：Neg
//       触发计数：1
      emit setDAParameters(1, true, 5, 1, 1, 4, 0, 0, 0, 0, 0, M*K, 0);

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
      emit setTTLParameters(0,true,  8, 0, 1, M+1, TTL0_T, 0, 1);

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
      emit setTTLParameters(5,true, 8, 1, 1, 1, TTL5_T, 0, TTL5_W);

//    6. 体周期信号（TTL4）
//       正脉宽=2ms
//       周期=相机去尾信号正脉宽TTL5_W+体间隔时间I
//       触发源： 软触发（SYN）
//       触发边沿：Pos
//       触发计数：1
//       猝发数目：体成像数目
      emit setTTLParameters(4,true, 1, 0, 1, N, TTL5_W+I, 0, 2);
}
