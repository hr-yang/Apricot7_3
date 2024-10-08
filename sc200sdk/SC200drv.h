//For Syperc SC200
#ifndef SC200DRVH
#define SC200DRVH

#ifdef __cplusplus 
extern "C" { 
#endif


/*
函数功能：打开设备
返回值：1 打开成功
        0 打开失败
*/
int SC200_Open();

/*
函数功能：关闭设备
*/
void SC200_Close();

/*
函数功能：通道位移设置
参数：channel 通道号 1~3
      pos 位移值，大小由台子量程决定
返回值：1 成功
        0 失败
*/
int SC200_SetPos(int channel, float pos);  

/*
函数功能：开闭环设置
参数：channel 通道号 1~3
      servo 开环为1，闭环0
返回值：1 成功
        0 失败
*/
int SC200_SetServo(int channel, int servo);


/*
函数功能：通道位移量读取
参数：channel 通道号 1~3
      pos  读取的位移值
返回值：1 成功
        0 失败
*/
int SC200_GetPos(int channel, float *pos);


#ifdef __cplusplus 
} 
#endif

#endif