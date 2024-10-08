#include "table.h"
#include "ui_table.h"
#include <QKeyEvent>
#include <QStandardPaths>
#include "qt_settings_widgetdatacontrol.h"
#include <QDebug>
#include <QGridLayout>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QtXlsx/QtXlsx>
#include <QStandardPaths>

Table::Table(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Table)
{
    ui->setupUi(this);

    ui->spinBox_Rows->installEventFilter(this);//为这个spinbox安装事件过滤器，监视此部件的回车事件
    ui->spinBox_Columns->installEventFilter(this);//为这个spinbox安装事件过滤器，监视此部件的回车事件

    ui->tableWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->tableWidget->setMinimumHeight(200);

    connect(ui->pushButton_SaveToExcel,&QPushButton::clicked,this,&Table::saveToExcelSlot);
    connect(ui->pushButton_ReadFromExcel,&QPushButton::clicked,this,&Table::readFromExcelSlot);

    p_table=ui->tableWidget;
}

Table::~Table()
{
    delete ui;
}

bool Table::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==ui->spinBox_Rows||obj==ui->spinBox_Columns)
    {
        if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *keyEvent=static_cast<QKeyEvent *>(event);
            if(keyEvent->key()==Qt::Key_Return)//回车键按下
            {
                ui->tableWidget->setRowCount(ui->spinBox_Rows->value());
                ui->tableWidget->setColumnCount(ui->spinBox_Columns->value());
                return true;
            }
            else
                return false;
        }
        else
            return false;
    }
    else
        return Table::eventFilter(obj,event);
}

void Table::saveToExcelSlot()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, tr("Save Excel"),
                                            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), tr("Excel File(*.xlsx)"));
    if (fileName.isEmpty())
    {
        qDebug()<<"load file fails";
        return;
    }
        QXlsx::Document xlsx(fileName);
        int tableR = ui->tableWidget->rowCount();
        int tableC = ui->tableWidget->columnCount();

        //获取表头写做第一行
        for (int j=0; j<tableC; j++)
            if ( ui->tableWidget->horizontalHeaderItem(j) != nullptr )
                xlsx.write(1, j+1,ui->tableWidget->horizontalHeaderItem(j)->text());//注意xlsx文件的起始行列都从1开始

        //写数据
        for (int i=0; i<tableR; i++)
        {
            for (int j=0; j<tableC; j++)
            {
                if ( ui->tableWidget->item(i,j) != nullptr )
                {
                    xlsx.write(i+2, j+1,ui->tableWidget->item(i,j)->text());//注意xlsx文件的起始行列都从1开始
                }
            }
        }
        xlsx.saveAs(fileName);//写完之后一定要保存
}

void Table::readFromExcelSlot()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this, tr("Open Excel"),
                                            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), tr("Excel File(*.xls *.xlsx)"));
    if (fileName.isEmpty())
    {
        qDebug()<<"read file fails";
        return;
    }
    QXlsx::Document xlsx(fileName);

    int nStartRow    = xlsx.dimension().firstRow();    //第一行有效数据的起始位置
    int nStartColumn = xlsx.dimension().firstColumn(); //第一列有效数据的起始位置
    int nLastRow    = xlsx.dimension().lastRow();
    int nLastColumn = xlsx.dimension().lastColumn();
    int nRowCount    = nLastRow-nStartRow+1;       //获取行数
    int nColumnCount = nLastColumn-nStartColumn+1;    //获取列数


    //先把table的内容清空
    ui->tableWidget->clear();
    for (int n=0; n<ui->tableWidget->columnCount(); n++)
    {
        ui->tableWidget->removeColumn(0);
    }

    //根据Excel设置表尺寸
    ui->spinBox_Rows->setValue(nRowCount-1);
    ui->spinBox_Columns->setValue(nColumnCount);
    ui->tableWidget->setRowCount(nRowCount-1);//第一行为表头
    ui->tableWidget->setColumnCount(nColumnCount);

    //获取excel中的第一行数据作为表头
    QStringList headerList;
    for (int n = nStartColumn; n<=nLastColumn; n++ )
    {
        QVariant temp=xlsx.read(nStartRow,n);
        if ( !temp.isNull() )
            headerList<<temp.toString();
    }
    //重新创建表头
    ui->tableWidget->setColumnCount(nColumnCount);
    ui->tableWidget->setHorizontalHeaderLabels(headerList);

    //插入数据
    for(int i=0;i<nRowCount;i++)//行列都从0开始，否则会卡死！！！
    {
        for(int j=0; j<nColumnCount; j++)
        {
            //tableWidget_DA->setItem(i,j, new QTableWidgetItem(xlsx.read(nStartRow+i+1,nStartColumn+j).toString()));  //会读入公式
            if (QXlsx::Cell *cell=xlsx.cellAt(nStartRow+i+1, nStartColumn+j))
                ui->tableWidget->setItem(i,j, new QTableWidgetItem(cell->value().toString()));
        }
    }

    QMessageBox::information(nullptr, "Table", "read sucess", QMessageBox::Yes, QMessageBox::Yes);
}
