#ifndef MOLISDIALOG_H
#define MOLISDIALOG_H

#include <QDialog>
#include "Global.h"

namespace Ui {
class MolisDialog;
}

class MolisDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MolisDialog(QWidget *parent = nullptr);
    ~MolisDialog();

private slots:
    void on_pushButtonCalc_clicked();

    void on_pushButtonExport_clicked();

    void on_pushButton_Update_clicked();

private:
    Ui::MolisDialog *ui;

signals:
    void setTTLParameters(int ch, bool enable, TrigSource trigSource, TrigEdge trigEdge, int trigCount, int burstNumber, double period, double phase, double duty);
    void setDAParameters(int ch, bool enable, TrigSource trigSource, TrigEdge trigEdge,int trigCount, WaveType waveType, int burstNumber, double period, double phase, double duty, int step, int readLen, double dma_period);

};

#endif // MOLISDIALOG_H
