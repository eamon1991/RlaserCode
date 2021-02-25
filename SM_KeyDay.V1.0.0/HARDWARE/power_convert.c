#include "sys.h"

void Set_LD_Volt(u8 power)  //根据功率设定DA值
{
	float v;
	
	float Cmax = (float)(info_1.Para.Imax_set/10.0);
	float Cmin = 0;
	
	//v = 23/8.0*(0.02*((Cmax-Cmin)/100.0*power+Cmin)+0.005);
	
	v = 15/8.0*(0.02*((Cmax-Cmin)/100.0*power+Cmin)+0.005*(Cmax-Cmin)/20.0);
		
	v	=	(float)(((u16)(v*100))/100.0);
	
	v = v*1.11;
		
	TLC5620_Set_DA(LD_Channel,v);
}

