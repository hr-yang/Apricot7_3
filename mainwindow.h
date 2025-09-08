#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void createActions();
    void createMenus();
    void createToolBars();
private:
      Ui::MainWindow *ui;
    QProcess myProcess;

    //action
    QAction *motorizedAction;
    QAction *nanoAction;
    QAction *fpgaSettingAction;
    QAction *stimulateAction;
    QAction *laserCtrlAction;
    QAction *sigmaCtrlAction;
    QAction *sigmaCtrl2Action;
    QAction *sigmaCtrl3Action;

    QAction *aboutAction;

    //menu
    QMenu *windowMenu;
    QMenu *helpMenu;
    //ToolBar
    QToolBar *windowTool;

    QHash<QString,void*> hashChildWindow; //记录子窗口句柄的数据结构


private slots:

    void OpenFPGA_SettingSlot();
    void OpenStimulateSlot();
    void OpenNanoStageSlot();
    void OpenMotorizedStageSlot();
    void OpenLaserCtrlSlot();
    void OpenSigmaCtrlSlot();
    void OpenSigmaCtrl2Slot();
    void OpenSigmaCtrl3Slot();
    void AboutSlot();

    void motorizedDialogStartExposureSlot();

    void recieveCloseSlot(const QString &);//接受子窗口关闭信号，注销相应子窗口句柄

    void showFinished(int exitCode, QProcess::ExitStatus exitStatus);

signals:
    void sendStartExposureSgn();
};

#endif // MAINWINDOW_H



