#ifndef STIMULATE_H
#define STIMULATE_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QSettings>

namespace Ui {
class stimulate;
}

class stimulate : public QDialog
{
    Q_OBJECT

public:
    explicit stimulate(QWidget *parent = 0);
    ~stimulate();


    void closeEvent(QCloseEvent *event);

private:


    QSerialPort *serial;

    QSettings* settings;

    Ui::stimulate *ui;

    char  isClose;

    //bool eventFilter(QObject *obj, QEvent *event);//事件过滤器，实现spinbox的回车键发送

private slots:

        //光电刺激
        void openComButtonSlot();
        void readSerialDataSlot();

        void ledModeButtonsClicked();
        void brightSliderReleaseSlot();
        void setMode1ButtonSlot();
        void resetButtonSlot();
        void runOnceSlot();



signals:
        void sendCloseSgn(const QString &);//向主窗口发送关闭信号，让主窗口消除注册的句柄
};

#endif // STIMULATE_H
