#include "mytcpsocket.h"
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QTime>
#include <QEventLoop>
#include <QTimer>


MyTcpSocket::MyTcpSocket(QObject *parent) : QObject(parent),socket(NULL)
{
    memset(&pmc400Status,0,256);
}

MyTcpSocket::~MyTcpSocket()//析构函数不是在子线程中执行
{
    if(socket!=nullptr)
    {
        delete socket;
    }
}

void MyTcpSocket::connect()//在子线程中执行
{
    tcpQueue.clear();

    if(socket!=nullptr)
    {
        delete socket;
    }
    socket = new QTcpSocket;


    //取消已有的连接
    socket->abort();
    //连接服务器
    socket->connectToHost("192.168.1.101", 80);

    //等待连接成功
    if(!socket->waitForConnected(1000))
    {
       // QMessageBox::information(NULL);//QMessageBox只可以在gui线程（主线程）使用
        emit sendMessageBox("connect failed!");
        isConnect=0;

        qDebug()<<"connect thread id: "<<QThread::currentThreadId();

        return;
    }

    QObject::connect(socket, &QTcpSocket::readyRead, this, &MyTcpSocket::socketReadDataSlot);
    QObject::connect(socket, &QTcpSocket::disconnected, this, &MyTcpSocket::socketDisconnectedSlot);

    qDebug()<<"connect thread id: "<<QThread::currentThreadId();
    emit sendMessageBox("connect success!");
    isConnect=1;

    while(1)
    {

        if(isConnect==0)  return;

        if(tcpQueue.isEmpty())
        {
            socket->write("TOCK");
        }
        else
        {
            socket->write(tcpQueue.dequeue().toLatin1().data());
        }
        socket->flush();



        //qDebug()<<"time:"<<QDateTime::currentDateTime().toMSecsSinceEpoch(); //毫秒级



        QEventLoop loop;
        QTimer::singleShot(100, Qt::PreciseTimer,&loop, SLOT(quit()));//非阻塞延时
        loop.exec();//必须使用事件循环来延时，事件循环会从队列中取出信号，别的槽才会有机会被调用


    }
}


void MyTcpSocket::disconnect()//在子线程中执行
{
    isConnect=0;
    socket->disconnectFromHost();

    qDebug()<<"sockDisconnected thread id: "<<QThread::currentThreadId();
}


void MyTcpSocket::socketDisconnectedSlot()//在子线程中执行
{
     emit sendMessageBox("disconnected!");
     isConnect=0;

     qDebug()<<"sockDisconn thread id: "<<QThread::currentThreadId();
}



void MyTcpSocket::socketReadDataSlot()//在子线程中执行
{
    //读取缓冲区数据
    char msg[256];
    if(socket->read((char *)(&msg),sizeof(msg))==256)
    {
        memcpy(&pmc400Status,&msg,256);
    }

    //qDebug()<<"sockRead thread id: "<<QThread::currentThreadId();
}



