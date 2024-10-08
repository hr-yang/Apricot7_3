#pragma once
#include "nanostage.h"


class SC200Channel1 :	public NanoStage
{
public:
    bool ConnectStage();
    bool DisconnectStage();
    bool SetAxisServo(int);
    bool MoveAxis(double value);
    double ReadAxisPosition();
	
};

class SC200Channel2 :	public NanoStage
{
public:
    bool SetAxisServo(int);
    bool MoveAxis(double value);
    double ReadAxisPosition();

};
