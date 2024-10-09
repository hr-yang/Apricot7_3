#ifndef MOLISDIALOG_H
#define MOLISDIALOG_H

#include <QDialog>

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

private:
    Ui::MolisDialog *ui;
};

#endif // MOLISDIALOG_H
