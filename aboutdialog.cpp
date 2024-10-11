#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // 指定文件：
    QFile inputFile(":/log/update_log.txt");
    // 只读打开：
    inputFile.open(QIODevice::ReadOnly);
    // 文本流：
    QTextStream in(&inputFile);
    //必须加，否则含有中文时乱码
    in.setCodec(QTextCodec::codecForName("utf-8"));
    // 将文本流读取到字符串中：
    QString line = in.readAll();
    // 关闭文本流：
    inputFile.close();

    ui->textEdit->setText(line);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
