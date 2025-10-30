#-------------------------------------------------
#
# Project created by QtCreator 2018-10-15T23:55:29
#
#-------------------------------------------------

QT       += core gui charts xlsx

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network serialport axcontainer

TARGET = Apricot
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS  #QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    molisdialog.cpp \
    motorizedstage.cpp \
    nanostage.cpp \
    PMC400.cpp \
    SC200.cpp \
    stimulate.cpp \
    fpga_setting.cpp \
    qt_settings_widgetdatacontrol.cpp \
    laserctrl.cpp \
    mytcpsocket.cpp \
    fpgachannel.cpp \
    fpga_ttl.cpp \
    mychartdata.cpp \
    kokidialog.cpp \
    aboutdialog.cpp \
    quiwidget.cpp \
    switchbutton.cpp \
    table.cpp




HEADERS += \
    Global.h \
        mainwindow.h \
    fpga_setting.h \
    molisdialog.h \
    switchbutton.h \
    motorizedstage.h \
    nanostage.h \
    PMC400.h \
    SC200.h \
    stimulate.h \
    qt_settings_widgetdatacontrol.h \
    laserctrl.h \
    mytcpsocket.h \
    fpgachannel.h \
    fpga_ttl.h \
    mychartdata.h \
    kokidialog.h \
    aboutdialog.h \
    quiwidget.h \
    table.h





FORMS += \
        mainwindow.ui \
    molisdialog.ui \
    stimulate.ui \
    nanostage.ui \
    motorizedstage.ui \
    laserctrl.ui \
    fpgachannel.ui \
    fpga_ttl.ui \
    kokidialog.ui \
    aboutdialog.ui \
    table.ui



RESOURCES += \
    mainwindow.qrc \
    qss.qrc


UI_DIR=./ui  ###规定UI文件编译后的存放位置   ./当前编译目录    ，彻底解决使用 design 修改 ui界面编译后界面未更新问题
              ###导致ui_*.h文件没有更新的原因是源代码中#include ui_*.h的位置和实际生成的位置不同，引用的是老的ui_*.h


RC_ICONS = "./icons/win.ico"


win32: LIBS += -L$$PWD/sc200sdk/ -lsc200drv

INCLUDEPATH += $$PWD/sc200sdk
DEPENDPATH += $$PWD/sc200sdk
