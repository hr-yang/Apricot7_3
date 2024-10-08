#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QThread>
#include "quiwidget.h"
#include <QFile>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QFile file(":/qss/silvery.css");
//    if (file.open(QFile::ReadOnly)) {
//        QString qss = QLatin1String(file.readAll());
//        QString paletteColor = qss.mid(20, 7);
//        a.setPalette(QPalette(QColor(paletteColor)));
//        a.setStyleSheet(qss);
//        file.close();
//    }

    MainWindow w;
    w.show();

    qDebug()<<"main thread id: "<<QThread::currentThreadId();

    return a.exec();
}
