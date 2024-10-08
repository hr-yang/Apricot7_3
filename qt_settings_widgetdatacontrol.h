#ifndef QT_SETTINGS_WIDGETDATACONTROL_H
#define QT_SETTINGS_WIDGETDATACONTROL_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDial>
#include <QSlider>
#include <QAbstractSlider>
#include <QTableWidget>

#include <QLabel>
#include <QTextBrowser>
#include <QLCDNumber>
#include <QProgressBar>

#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>

#include <QSettings>

class Qt_Settings_WidgetDataControl
{
public:
    enum controlFlagsEnum
        {
            controlFlagsRecursion       =       0x1,

            controlFlagsPos             =       0x2,
            controlFlagsSize            =       0x4,
            controlFlagsEnabled         =       0x8,

            controlFlagsQLineEdit       =      0x10,
            controlFlagsQTextEdit       =      0x20,
            controlFlagsQSpinBox        =      0x40,
            controlFlagsQDoubleSpinBox  =      0x80,
            controlFlagsQTimeEdit       =     0x100,
            controlFlagsQDateEdit       =     0x200,
            controlFlagsQDateTimeEdit   =     0x400,
            controlFlagsQDial           =     0x800,
            controlFlagsQSlider         =    0x1000,
            controlFlagsQAbstractSlider =    0x2000,
            controlFlagsQTableWidget    =    0x4000,
            controlFlagsInputWidgets    =    0xfff0,

            controlFlagsQLabel          =   0x10000,
            controlFlagsQTextBrowser    =   0x20000,
            controlFlagsQLCDNumber      =   0x40000,
            controlFlagsQProgressBar    =   0x80000,
            controlFlagsDisplayWidgets  =   0xf0000,

            controlFlagsQRadioButton    = 0x1000000,
            controlFlagsQCheckBox       = 0x2000000,
            controlFlagsQComboBox       = 0x4000000,
            controlFlagsQGroupBox       = 0x8000000,
            controlFlagsButtons         = 0xf000000,

            controlFlgasWidgets         = 0xf00fff1,
            controlFlagsAll             = 0xfffffff

        };

    public:
        Qt_Settings_WidgetDataControl();


        static void save(QSettings *settings, const QWidget *widget, const controlFlagsEnum &controlFlags);

        static void read(QSettings *settings, QWidget *widget, const controlFlagsEnum &controlFlags);


};

#endif // QT_SETTINGS_WIDGETDATACONTROL_H
