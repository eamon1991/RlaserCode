#include "sys.h"

#define IIC_SCL    PBout(6) 
#define IIC_SDA    PBout(7)  
#define READ_SDA   PBin(7) 
 
#define Config_Register		0x00  //配置寄存器
#define ShuntV_Register		0x01 //
#define BusVolt_Register	0x02 //总线电压寄存器
#define Power_Register		0x03 //功率寄存器
#define Current_Register	0x04 //电流寄存器
#define Calibrat_Register	0x05 //校准寄存器

static GPIO_InitTypeDef GPIO_InitStructure_IIC;

void SDA_OUT()
{
	GPIO_InitStructure_IIC.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure_IIC.GPIO_Mode =  GPIO_Mode_Out_OD;//开漏
	GPIO_InitStructure_IIC.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure_IIC);
}

void SDA_IN()
{
	GPIO_InitStructure_IIC.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure_IIC.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure_IIC.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure_IIC);
}

static void IIC_Delay()
{
	//delay_us(2);
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();

}

static void IIC_Start(void)	
{
	SDA_OUT(); 
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	//delay_us(4);
	IIC_Delay();
 	IIC_SDA=0;
	//delay_us(4);
	IIC_Delay();
	IIC_SCL=0;
}	  

static void IIC_Stop(void)
{
	SDA_OUT();
	IIC_SCL=0;
	IIC_SDA=0;
	IIC_Delay();
 	//delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;
	IIC_Delay();
	//delay_us(4);							   	
}
      
static u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();   
	IIC_SDA=1;IIC_Delay();//delay_us(1);	   
	IIC_SCL=1;IIC_Delay();//delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;	   
	return 0;  
} 

static void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	IIC_Delay();
	//delay_us(2);
	IIC_SCL=1;
	IIC_Delay();
	//delay_us(2);
	IIC_SCL=0;
}
  
static void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	IIC_Delay();
	//delay_us(2);
	IIC_SCL=1;
	IIC_Delay();
	//delay_us(2);
	IIC_SCL=0;
}			
	  
static void IIC_Send_Byte(u8 txd)
{                        
	u8 t;   
	SDA_OUT(); 	    
  IIC_SCL=0;
  for(t=0;t<8;t++)
  {              
		IIC_SDA=(txd&0x80)>>7;
    txd<<=1; 	  
		IIC_Delay();
		//delay_us(2);
		IIC_SCL=1;
		IIC_Delay();
		
		//delay_us(2); 
		IIC_SCL=0;	
		IIC_Delay();
		//delay_us(2);
  }	 
} 	

static u8 IIC_Read_Byte(unsigned char ack)	 
{
	unsigned char i,receive=0;
	SDA_IN();
  for(i=0;i<8;i++ )
	{
		IIC_SCL=0; 
		IIC_Delay();
    //delay_us(2);
		IIC_SCL=1;
    receive<<=1;
    if(READ_SDA)receive++; 
		IIC_Delay();		
		//delay_us(1); 
  }					 
  if (!ack)	IIC_NAck();
  else	IIC_Ack(); 
	
	return receive;
}


u8 drive_tab[16]={0x80,0x82,0x84,0x86,0x88,0x8A,0x8C,0x8E,0x90,0x92,0x94,0x96,0x98,0x9A,0x9C,0x9E};
u8 drive_num=0;
u8 drive[16];

/* CalibrationRegister_Value = 0.04096/(currentLSB*Rshunt);
	minCurrentLSB = maxExpectedCurrent/2^15;*/
void INA220_Init(void)	  //采集电流IC 初始化
{	
	u8 i;
	const u16 calculate_set = (u16)(0.04096/(0.001*0.02));
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;//开漏
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	IIC_SCL=1;
	IIC_SDA=1;
	
	for(i=0;i<16;i++)
	{
		if(INA220_ReadOneWord(drive_tab[i],0) != 0xFFFF)	
		{
			drive[drive_num++] = drive_tab[i];
			//printf("%04x ",drive_tab[i]);
		}
	}
	if(drive_num >= 6) drive_num = 6;
	
	for(i=0;i<drive_num;i++)	
	{
		INA220_WriteOneWord(drive[i],0,0x39F7);
		INA220_WriteOneWord(drive[i],5,calculate_set);
	}
}

u16 INA220_ReadOneWord(u8 device,u8 ReadAddr)
{				  
	u16 temp=0;		  	    																 
  IIC_Start();  
	IIC_Send_Byte(device&0xFE);	  
	IIC_Wait_Ack();
	IIC_Send_Byte(ReadAddr);
	IIC_Wait_Ack();		 
	IIC_Start();  	 	   
	IIC_Send_Byte(device|0x01);  		   
	IIC_Wait_Ack();	 
  temp	=	IIC_Read_Byte(1);	
	temp	=	(temp<<8)|IIC_Read_Byte(0);	
  IIC_Stop();   
	return temp;
}

void INA220_WriteOneWord(u8 device,u8 WriteAddr,u16 DataToWrite)
{				   	  	    																 
	IIC_Start();  
	IIC_Send_Byte(device&0xFE);
	IIC_Wait_Ack();
	IIC_Send_Byte(WriteAddr); 
	IIC_Wait_Ack();	
	IIC_Send_Byte(((DataToWrite>>8)&0xFF)); 						   
	IIC_Wait_Ack(); 
	IIC_Send_Byte(DataToWrite&0xFF); 						   
	IIC_Wait_Ack(); 	
  IIC_Stop();
}
//
float INA_ReadCurrent(u8 drive_select)  //读取电流函数  drive_select芯片地址
{
	u16 currentTemp;
	currentTemp = INA220_ReadOneWord(drive_select,Current_Register);
	if(currentTemp&0x8000)	currentTemp &= 0x7FFF; //32767
	if(Resistance ) {return (float)(currentTemp*0.002*13/14);} //0.01电阻
	else   {return (float)(currentTemp*0.001);}  //0.02电阻
}

float INA_ReadVoltage(u8 drive_select)  //读取电压函数  drive_select芯片地址
{
	u8 i;
	u16 ValueTemp=0,ValueMax=0,ValueMin=0,ValueSum=0,ValueAverage=0;
	u16 Vbus;
	float Vin;
	
	for(i=0;i<10;i++)
	{
		ValueTemp = INA220_ReadOneWord(drive_select,BusVolt_Register);
		if(ValueTemp < ValueMin)	ValueMin = ValueTemp;
		else if(ValueTemp > ValueMax) ValueMax = ValueTemp;
		ValueSum +=ValueTemp;
	}
	
	ValueAverage = (ValueSum-ValueMin-ValueMax)/8;
	
	Vbus = (ValueAverage>>3)*4;
	Vin = (float)(Vbus*(1.0/1000));
	
	return Vin;
}

void Get_Current()  //读取设备电流
{
	u8 i;
	float *p = &info_1.current1;
	for(i=0;i<drive_num;i++)	*p++ = INA_ReadCurrent(drive[i]);
	Average_Current();  //0.25S内50次电流平均 
	//for(i=0;i<drive_num;i++)	*p++ = INA_ReadVoltage(drive[i])/0.1;
	//printf("%.2f %.2f %.2f %.2f %.2f %.2f\r\n",info_1.current1,info_1.current2,info_1.current3,info_1.current4,info_1.current5,info_1.current6);
}
u16 Average_I[16]; 
void Average_Current()  //0.5S内50次电流平均
{
	u16 ix,im;
	u16 andI;
	float currentS[16];
	static u16 Currents[16][50];
	
	currentS[0]=info_1.current1;
	currentS[1]=info_1.current2;
	currentS[2]=info_1.current3;
	currentS[3]=info_1.current4;
	currentS[4]=info_1.current5;	
	currentS[5]=info_1.current6;
	currentS[6]=info_1.current7;
	currentS[7]=info_1.current8;
	currentS[8]=info_1.current9;
	currentS[9]=info_1.current10;
	currentS[10]=info_1.current11;
	currentS[11]=info_1.current12;
	currentS[12]=info_1.current13;
	currentS[13]=info_1.current14;
	currentS[14]=info_1.current15;
	currentS[15]=info_1.current16;
  if(drive_num<=16)
	{
	for(ix=0;ix<drive_num;ix++)
  {	
		andI=0;
		for(im=0;im<49;im++)
		{
			Currents[ix][im] = Currents[ix][im+1];
			andI=andI+Currents[ix][im];
		}		
		Currents[ix][49] = (u16)((currentS[ix])*10); //电流1-num 
		andI=andI+Currents[ix][49];
		Average_I[ix]=andI/50*1.09;
	}
  }
}



