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
