#include "PMC400.h"
#include <QDebug>

//static PMC400 pmc400;
/********************************X轴********************************************************/

bool PMC400AxisX::InitStage()
{
	//pmc400.PMC400Init();	
	qDebug()<<"run pmc400init"<<endl;
	return 0;
}

bool PMC400AxisX::ConnectStage()
{
	qDebug()<<"run pmc400connect"<<endl;
	return 0;//pmc400.Connect("192.168.1.101",80);	
}

bool PMC400AxisX::DisconnectStage()
{
	return 0;
}

void PMC400AxisX::SetParamters()
{
	x_convert_=200*64/2; // 丝杆导程1圈-2mm     1mm-0.5圈    -0.5* (200 *8)    cnt
	//pmc400.PMC400_Set_ScrewLead("Y轴", 2000000);//丝杆导程（纳米）  1圈-2mm
	//pmc400.PMC400_Set_MicroSteps("Y轴", 64);//64细分
	//pmc400.PMC400_Set_Resolution_Ratio("Y轴", 10000);//光栅尺分辨率（注：此处为脉冲/毫米，即1MM对应多少脉冲）,开环请设为0
	//pmc400.PMC400_Save_Setup();//注意保存设置
}

bool PMC400AxisX::MoveAxis(double value)
{
     Q_UNUSED(value);
	//速度设置
	//pmc400.PMC400_Set_StartSpeed("Y轴", 0);//初速度（mm/s）
	//pmc400.PMC400_Set_AccSpeed("Y轴", 10*x_convert_);//加速度（mm/s2）
	//pmc400.PMC400_Set_FinalSpeed("Y轴",10*x_convert_);//中(匀速)速度（mm/s）

	//pmc400.PMC400_Set_OpenMoveDistance("Y轴",value*x_convert_);//移动距离（脉冲），开环
	//pmc400.PMC400_StartMove();//在以上设置完毕之后,调用此函数开始移动

	return 0;
}

double PMC400AxisX::ReadAxisPosition()
{
	return 0;//(double)//(pmc400.PMC400_Get_PositionEncode("Y轴"))/(double)(pmc400.PMC400_Get_Resolution_Ratio("Y轴"));
}

bool PMC400AxisX::ReadAxisLimit(int dir)
{
    Q_UNUSED(dir);
	//if(dir==1)
	//{
	//	//if(pmc400.PMC400_Get_El_Left_Data("Y轴")) return 1;
	//}
	//else
	//{
	//	//if(pmc400.PMC400_Get_El_Right_Data("Y轴")) return 1;
	//}	

	return 0;
}

void PMC400AxisX::Stop()
{
	//pmc400.PMC400_StopMove();
	//pmc400.PMC400_Stop();
}

/********************************Y轴********************************************************/

void PMC400AxisY::SetParamters()
{
	y_convert_ = 200 * 8 / 2;
	//pmc400.PMC400_Set_ScrewLead("Z轴", 2000000);//丝杆导程（纳米）
	//pmc400.PMC400_Set_MicroSteps("Z轴", 8);
	//pmc400.PMC400_Set_Resolution_Ratio("Z轴", 0);//光栅尺分辨率（注：此处为脉冲/毫米，即1MM对应多少脉冲）,开环请设为0
	//pmc400.PMC400_Save_Setup();//注意保存设置
}

bool PMC400AxisY::MoveAxis(double value)
{
    Q_UNUSED(value);
	//速度设置
	//pmc400.PMC400_Set_StartSpeed("Z轴", 0);//初速度（mm/s）
	//pmc400.PMC400_Set_AccSpeed("Z轴", 10*y_convert_);//加速度（mm/s2）
	//pmc400.PMC400_Set_FinalSpeed("Z轴",10*y_convert_);//中(匀速)速度（mm/s）

	//pmc400.PMC400_Set_OpenMoveDistance("Z轴",value*y_convert_);//移动距离（脉冲），开环
	//pmc400.PMC400_StartMove();//在以上设置完毕之后,调用此函数开始移动

	return 0;
}

double PMC400AxisY::ReadAxisPosition()
{
	return 0;
}

bool PMC400AxisY::ReadAxisLimit(int dir)
{
    Q_UNUSED(dir);
	/*if(dir==1)
	{
		if(pmc400.PMC400_Get_El_Right_Data("Z轴")) return 1;
	}
	else
	{
		if(pmc400.PMC400_Get_El_Left_Data("Z轴")) return 1;
	}	*/
	return 0;
}

void PMC400AxisY::Stop()
{
	/*pmc400.PMC400_StopMove();
	pmc400.PMC400_Stop();*/
}

/********************************Z轴********************************************************/

void PMC400AxisZ::SetParamters()
{
	z_convert_ = 200 * 200 / 1.5785f;
	//pmc400.PMC400_Set_ScrewLead("U轴", 1578500);//丝杆导程（纳米）
	//pmc400.PMC400_Set_MicroSteps("U轴", 200);	
	//pmc400.PMC400_Set_Resolution_Ratio("U轴", 10000);//光栅尺分辨率（注：此处为脉冲/毫米，即1MM对应多少脉冲）,开环请设为0
	//pmc400.PMC400_Save_Setup();//注意保存设置
}

bool PMC400AxisZ::MoveAxis(double value)
{
     Q_UNUSED(value);
	//pmc400.PMC400_Set_StartSpeed("U轴",0);//初速度
	//pmc400.PMC400_Set_AccSpeed("U轴", 10 * z_convert_);//加速度
	//pmc400.PMC400_Set_FinalSpeed("U轴", 10*z_convert_);//中速度

	//pmc400.PMC400_Set_OpenMoveDistance("U轴",value*z_convert_);//移动距离（脉冲），开环
	//pmc400.PMC400_StartMove();//在以上设置完毕之后,调用此函数开始移动

	return 0;
}

double PMC400AxisZ::ReadAxisPosition()
{
	return 0;//(double)(pmc400.PMC400_Get_PositionEncode("U轴") )/(double)( pmc400.PMC400_Get_Resolution_Ratio("U轴"));
}

bool PMC400AxisZ::ReadAxisLimit(int dir)
{
    Q_UNUSED(dir);
	/*if(dir==1)
	{
		if(pmc400.PMC400_Get_El_Right_Data("U轴")) return 1;
	}
	else
	{
		if(pmc400.PMC400_Get_El_Left_Data("U轴")) return 1;
	}	*/
	return 0;
}

void PMC400AxisZ::Stop()
{
	/*pmc400.PMC400_StopMove();
	pmc400.PMC400_Stop();*/
}

/********************************U轴********************************************************/


void PMC400AxisU::SetParamters()
{
	u_convert_=20*200/2.5;//此电机20个脉冲一圈   1600
//	pmc400.PMC400_Set_SpeedRatio("X轴", 250);/*设置旋转台转速比(n*百分之一度/1圈)*/
//	pmc400.PMC400_Set_MicroSteps("X轴", 200);//设置指定轴的细分数 范围 1-512 ，可用 PMC400_Get_MicroSteps 查询
//	pmc400.PMC400_Set_Resolution_Ratio("X轴", 0);//光栅尺分辨率（注：此处为脉冲/毫米，即1MM对应多少脉冲）,开环请设为0
//	pmc400.PMC400_Save_Setup();//注意保存设置
}

bool PMC400AxisU::MoveAxis(double value)
{
     Q_UNUSED(value);
	//pmc400.PMC400_Set_StartSpeed("X轴", 0*u_convert_);//初速度（度/秒）
	//pmc400.PMC400_Set_AccSpeed("X轴",   3*u_convert_);//加速度（度/平方秒）
	//pmc400.PMC400_Set_FinalSpeed("X轴", 3*u_convert_);//终速度（度/秒）
	//pmc400.PMC400_Set_OpenMoveDistance("X轴",value*u_convert_);//移动距离（脉冲），开环
	//pmc400.PMC400_StartMove();//在以上设置完毕之后,调用此函数开始移动

	return 0;
}

double PMC400AxisU::ReadAxisPosition()
{
	return 0;
}

bool PMC400AxisU::ReadAxisLimit(int dir)
{
    Q_UNUSED(dir);
	return 0;
}

void PMC400AxisU::Stop()
{
	//pmc400.PMC400_StopMove();
	//pmc400.PMC400_Stop();
}
