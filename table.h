#ifndef TABLE_H
#define TABLE_H

#include <QWidget>
#include <QTableWidget>

namespace Ui {
class Table;
}

class Table : public QWidget
{
    Q_OBJECT

public:
    explicit Table(QWidget *parent = nullptr);
    ~Table();

    QTableWidget *p_table;
private:
    Ui::Table *ui;

    bool eventFilter(QObject *obj, QEvent *event);//事件过滤器，实现spinbox的回车键发送

public slots:
    void saveToExcelSlot();
    void readFromExcelSlot();
};

#endif // TABLE_H
