#pragma once
#include "motorizedstage.h"




class PMC400AxisX:public MotorizedStage
{
public:
	bool InitStage();
	bool ConnectStage();
	bool DisconnectStage();
	void SetParamters();
	bool MoveAxis(double value);
	double ReadAxisPosition();
	bool ReadAxisLimit(int dir);
	void Stop();
private:
	double x_convert_;
	//PMC400 pmc400;//整个工程只能生成一个实例
};

class PMC400AxisY:public MotorizedStage
{
public:
	void SetParamters();
	bool MoveAxis(double value);
	double ReadAxisPosition();
	bool ReadAxisLimit(int dir);
	void Stop();
private:
	double y_convert_;
	//PMC400 pmc400;
};

class PMC400AxisZ:public MotorizedStage
{
public:
	void SetParamters();
	bool MoveAxis(double value);
	double ReadAxisPosition();
	bool ReadAxisLimit(int dir);
	void Stop();
private:
	double z_convert_;
	//PMC400 pmc400;
};

class PMC400AxisU:public MotorizedStage
{
public:
	void SetParamters();
	bool MoveAxis(double value);
	double ReadAxisPosition();
	bool ReadAxisLimit(int dir);
	void Stop();
private:
	double u_convert_;
	//PMC400 pmc400;
};
