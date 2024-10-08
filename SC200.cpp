#include "SC200.h"
//#include "SC200drv.h"

/////////////////////////////////////////////channel 1//////////////////////////////////////
bool SC200Channel1::ConnectStage()
{
    //return SC200_Open();
    return 1;
}

bool SC200Channel1::DisconnectStage()
{
    //SC200_Close();
    return 1;
}

bool SC200Channel1::SetAxisServo(int servo)
{
    //return SC200_SetServo(1,servo);
    return 1;
}

bool SC200Channel1::MoveAxis(double value)
{
    //return SC200_SetPos(1,value);
    return 1;
}

double SC200Channel1::ReadAxisPosition()
{
    float value;
    //SC200_GetPos(1,&value);
    return value;
}


/////////////////////////////////////////////channel 2//////////////////////////////////////

bool SC200Channel2::SetAxisServo(int servo)
{
    //return SC200_SetServo(2,servo);
    return 1;
}

bool SC200Channel2::MoveAxis(double value)
{
    //return SC200_SetPos(2,value);
    return 1;
}

double SC200Channel2::ReadAxisPosition()
{
    float value;
    //SC200_GetPos(2,&value);
    return value;

}
