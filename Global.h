#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>
#include <QMetaType>

enum TrigSource//与下位机保持一致
{
    SOFT=0,
    SYN =1,
    EXT0=2,
    EXT1=3,
    TTL0=4,
    TTL1=5,
    TTL2=6,
    TTL3=7,
    TTL4=8,
    TTL5=9,
};

enum TrigEdge//与下位机保持一致
{
    POS =0,
    NEG =1,
};

enum WaveType //与下位机保持一致
{
    ZERO=0,
    DIRECT=1,
    MIN=2,
    MAX=3,
    DOUBLE_TRIANGLE=4,
    SINGLE_TRIANGLE =5,
    STEP_WAVE=6,
    SQUARE_WAVE=7,
    BRAM_DA=8,
    DMA_DA=9,
};


Q_DECLARE_METATYPE(TrigSource)
Q_DECLARE_METATYPE(TrigEdge)
Q_DECLARE_METATYPE(WaveType)

#endif // GLOBAL_H
