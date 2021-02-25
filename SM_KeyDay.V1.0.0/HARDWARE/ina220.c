#include "sys.h"



#define Config_Register		0x00
#define ShuntV_Register		0x01
#define BusVolt_Register	0x02
#define Power_Register		0x03
#define Current_Register	0x04
#define Calibrat_Register	0x05

void INA220_Write_Register(u8 SlaveDevice,u8 reg,u16 dat)
{
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART( I2C1, ENABLE );
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	 
	I2C_Send7bitAddress( I2C1, SlaveDevice, I2C_Direction_Transmitter );					
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData( I2C1, reg );
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	

	I2C_SendData( I2C1, dat>>8 );
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));		
	I2C_SendData( I2C1, dat );
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));	

	I2C_GenerateSTOP( I2C1, ENABLE );		//IIC产生STOP条件	
}

u16	INA220_Read_Register( u8 slaveDevice, u8 reg_pointer)
{  
	u8 dat_receive[2];
	
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));								
	
	I2C_GenerateSTART( I2C1, ENABLE );																				
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));							

	I2C_Send7bitAddress( I2C1, slaveDevice, I2C_Direction_Transmitter );									
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));	

	I2C_SendData( I2C1, reg_pointer );  																		
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
			  
	I2C_GenerateSTART( I2C1, ENABLE );
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress( I2C1, slaveDevice, I2C_Direction_Receiver );
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	
//	I2C_AcknowledgeConfig( I2C1, DISABLE );	
//	I2C_GenerateSTOP( I2C1, ENABLE );
//	while (!(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)));	  
//	*buf_read = I2C_ReceiveData(I2C1);
//	I2C_AcknowledgeConfig( I2C1, ENABLE );

	I2C_AcknowledgeConfig(I2C1, DISABLE);
  I2C_GenerateSTOP(I2C1, ENABLE);

	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
	dat_receive[0] = I2C_ReceiveData(I2C1);	
	
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
	dat_receive[1] = I2C_ReceiveData(I2C1);

  I2C_AcknowledgeConfig(I2C1, ENABLE);
	
	return (u16)((dat_receive[0]<<8)|dat_receive[1]);
}


void ina220_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x0A;
	I2C_InitStructure.I2C_Ack = ENABLE;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;
	I2C_Init(I2C1,&I2C_InitStructure);
	
	I2C_Cmd(I2C1,ENABLE);
	
	/* CalibrationRegister_Value = 0.04096/(currentLSB*Rshunt); minCurrentLSB = maxExpectedCurrent/2^15;*/
	INA220_Write_Register(DriveModule1,Calibrat_Register,(u16)(0.04096/(0.001*0.05)));	//1mA,50m欧
	INA220_Write_Register(DriveModule8,Calibrat_Register,(u16)(0.04096/(0.001*0.05)));	//1mA,50m欧
}

float INA_ReadCurrent()
{
	u16 current = INA220_Read_Register(DriveModule1,Current_Register);
	if(current&0x8000)	current &= 0x7FFF;
	return (float)(current*0.001);
}

float INA_ReadVoltage()
{
	u8 i;
	u16 ValueTemp=0,ValueMax=0,ValueMin=0,ValueSum=0,ValueAverage=0;
	u16 Vbus;
	float Vin;
	
	for(i=0;i<10;i++)
	{
		ValueTemp = INA220_Read_Register(DriveModule1,BusVolt_Register);
		if(ValueTemp < ValueMin)	ValueMin = ValueTemp;
		else if(ValueTemp > ValueMax) ValueMax = ValueTemp;
		ValueSum +=ValueTemp;
	}
	ValueAverage = (ValueSum-ValueMin-ValueMax)/8;
	
	Vbus = (ValueAverage>>3)*4;
	Vin = (float)(Vbus*(1.0/1000));
	
	return Vin;
}

void Get_Current()
{
	
}
