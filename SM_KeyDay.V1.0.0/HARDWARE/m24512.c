#include "sys.h"


#define M24512_Address 0xA0

#define M24512_IIC_SCL    PBout(10) //SCL
#define M24512_IIC_SDA    PBout(11) //SDA	 
#define M24512_READ_SDA   PBin(11)  //����SDA 
#define M24512_CS	 PAout(8)

void load_para()
{	
	if(M24512_ReadWord(ParaSetAddr) == 0xFFFF)	
	{	//��һ���ϵ磬��ʼ������
		info_1.Para.red_set = 70;
		info_1.Para.pd1_set = 8;
		info_1.Para.pd2_set = 8;
		info_1.Para.Tover_set = 30;
		info_1.Para.Iover_set = 200;
		info_1.Para.Imax_set = 110;
		info_1.Para.alarm_screen = 0xFF;
		
		M24512_WriteWord(ParaRedAddr,info_1.Para.red_set);
		M24512_WriteWord(ParaPD1Addr,info_1.Para.pd1_set);
		M24512_WriteWord(ParaPD2Addr,info_1.Para.pd2_set);
		M24512_WriteWord(ParaToverAddr,info_1.Para.Tover_set);
		M24512_WriteWord(ParaIoverAddr,info_1.Para.Iover_set);
		M24512_WriteWord(ParaImaxAddr,info_1.Para.Imax_set);
		M24512_WriteWord(ParaScrAddr,info_1.Para.alarm_screen);
		M24512_WriteWord(LockFlagAddr,0);
		M24512_WriteWord(LockYearAddr,2017);
		M24512_WriteWord(LockMonthAddr,11);
		M24512_WriteWord(LockDayAddr,21);
		M24512_WriteWord(LockHourAddr,14);
		M24512_WriteWord(LockMinAddr,0);
		M24512_WriteWord(ErrGuideAddr,0x01);
		M24512_WriteWord(ParaSetAddr,0x01);
	}//�ϵ���ȡ������������
	info_1.Para.red_set = M24512_ReadWord(ParaRedAddr);
	info_1.Para.pd1_set = M24512_ReadWord(ParaPD1Addr);
	info_1.Para.pd2_set = M24512_ReadWord(ParaPD2Addr);
	info_1.Para.Tover_set = M24512_ReadWord(ParaToverAddr);
	info_1.Para.Iover_set = M24512_ReadWord(ParaIoverAddr);
	info_1.Para.Imax_set = M24512_ReadWord(ParaImaxAddr);
	info_1.Para.alarm_screen = M24512_ReadWord(ParaScrAddr);
	
	TLC5620_Set_DA(RED_Channel,(float)(info_1.Para.red_set/10.0));
	TLC5620_Set_DA(PD1_Channel,(float)(info_1.Para.pd1_set/10.0));
	TLC5620_Set_DA(PD2_Channel,(float)(info_1.Para.pd2_set/10.0));
	TLC5620_Set_DA(PD2_Channel,(float)(info_1.Para.pd2_set/10.0));
	TLC5620_Set_DA(LD_Channel,0);
}

/************************ M24512��ʼ�� *************************/
void M24512_Init(void)	
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	M24512_CS = 0;
	M24512_IIC_SCL=1;
	M24512_IIC_SDA=1;
	if(M24512_Check())	//�洢оƬ�Լ죬����0���ɹ�������1��ʧ��
	{
		info_1.Para.red_set = 70;
		info_1.Para.pd1_set = 8;
		info_1.Para.pd2_set = 8;
		info_1.Para.Tover_set = 30;
		info_1.Para.Iover_set = 200;
		info_1.Para.Imax_set = 110;
		info_1.Para.alarm_screen = 0xFF;
	}else	
	{
		load_para();
	}
}

/*******************************M24512_IIC����************************************/
static void M24512_SDA_IN()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void M24512_SDA_OUT()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void M24512_IIC_Start(void)	//����IIC��ʼ�ź�
{
	M24512_SDA_OUT(); 
	M24512_IIC_SDA=1;	  	  
	M24512_IIC_SCL=1;
	delay_us(4);
 	M24512_IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	M24512_IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  

static void M24512_IIC_Stop(void)	//����IICֹͣ�ź�
{
	M24512_SDA_OUT();
	M24512_IIC_SCL=0;
	M24512_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	M24512_IIC_SCL=1; 
	M24512_IIC_SDA=1;//����I2C���߽����ź�
	delay_us(4);							   	
}
      
static u8 M24512_IIC_Wait_Ack(void)	//�ȴ�Ӧ���źŵ�����1������Ӧ��ʧ�ܣ�0������Ӧ��ɹ�
{
	u8 ucErrTime=0;
	M24512_SDA_IN();      //SDA����Ϊ����  
	M24512_IIC_SDA=1;delay_us(1);	   
	M24512_IIC_SCL=1;delay_us(1);	 
	while(M24512_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			M24512_IIC_Stop();
			return 1;

		}
	}
	M24512_IIC_SCL=0;//ʱ�����0 	
	//delay_us(1);	
	return 0;  
} 

static void IIC_Ack(void)	//����ACKӦ��
{
	M24512_IIC_SCL=0;
	M24512_SDA_OUT();
	M24512_IIC_SDA=0;
	delay_us(2);
	M24512_IIC_SCL=1;
	delay_us(2);
	M24512_IIC_SCL=0;
}
  
static void M24512_IIC_NAck(void)	//������ACKӦ��		
{
	M24512_IIC_SCL=0;
	M24512_SDA_OUT();
	M24512_IIC_SDA=1;
	delay_us(2);
	M24512_IIC_SCL=1;
	delay_us(2);
	M24512_IIC_SCL=0;
}			
	  
static void M24512_IIC_Send_Byte(u8 txd)	//IIC����һ���ֽ�,���شӻ�����Ӧ��1����Ӧ��,0����Ӧ��	
{                        
	u8 t;   
	M24512_SDA_OUT(); 	    
  M24512_IIC_SCL=0;
  for(t=0;t<8;t++)
  {              
		M24512_IIC_SDA=(txd&0x80)>>7;
    txd<<=1; 	  
		delay_us(2); //��TEA5767��������ʱ���Ǳ����
		M24512_IIC_SCL=1;
		delay_us(2); 
		M24512_IIC_SCL=0;	
		delay_us(2);
  }	 
} 	

static u8 M24512_IIC_Read_Byte(unsigned char ack)	//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK  
{
	unsigned char i,receive=0;
	M24512_SDA_IN();//SDA����Ϊ����
  for(i=0;i<8;i++ )
	{
		M24512_IIC_SCL=0; 
    delay_us(2);
		M24512_IIC_SCL=1;
    receive<<=1;
    if(M24512_READ_SDA)receive++;   
		delay_us(1); 
  }					 
  if (!ack)	M24512_IIC_NAck();//����nACK
  else	IIC_Ack(); //����ACK  
	
	return receive;
}

/**************** M24512��ȡһ���ֽ����� *******************/
u8 M24512_ReadByte(u16 addr)
{
	u8 temp=0;		  	    																 
  M24512_IIC_Start();  
	M24512_IIC_Send_Byte(0XA0);	   
	M24512_IIC_Wait_Ack();
	M24512_IIC_Send_Byte((u8)((addr>>8)&0xFF));
	M24512_IIC_Wait_Ack();		 
  M24512_IIC_Send_Byte((u8)(addr&0XFF));   
	M24512_IIC_Wait_Ack();	
	M24512_IIC_Start();  	 	   
	M24512_IIC_Send_Byte(0XA1);       		   
	M24512_IIC_Wait_Ack();	 
  temp=M24512_IIC_Read_Byte(0);	
  M24512_IIC_Stop(); 
	return temp;
}

/************** M24512д��һ���ֽ����� **********************/
void M24512_WriteByte(u16 addr,u8 dat)
{
	M24512_IIC_Start();  
	M24512_IIC_Send_Byte(0XA0);	 
	M24512_IIC_Wait_Ack();
	M24512_IIC_Send_Byte(addr>>8);
	M24512_IIC_Wait_Ack();	   
  M24512_IIC_Send_Byte(addr); 
	M24512_IIC_Wait_Ack(); 	 										  		   
	M24512_IIC_Send_Byte((u8)(dat)); 						   						   
	M24512_IIC_Wait_Ack(); 	
  M24512_IIC_Stop();
	delay_ms(4);
}

/*************** M24512д��һ�������� *******************/
void M24512_WriteWord(u16 addr,u16 dat)
{
	M24512_IIC_Start();  
	M24512_IIC_Send_Byte(0XA0);	 
	M24512_IIC_Wait_Ack();
	M24512_IIC_Send_Byte(addr>>8);
	M24512_IIC_Wait_Ack();	   
  M24512_IIC_Send_Byte(addr); 
	M24512_IIC_Wait_Ack(); 	 										  		   
	M24512_IIC_Send_Byte((u8)(dat>>8)); 
	M24512_IIC_Wait_Ack(); 	 										  		   
	M24512_IIC_Send_Byte((u8)(dat)); 
	M24512_IIC_Wait_Ack();	
  M24512_IIC_Stop();
	delay_ms(10);
}
/************* M24512��ȡһ�������� *********************/
u16 M24512_ReadWord(u16 addr)
{
	u16 temp=0;		  	    																 
  M24512_IIC_Start();  
	M24512_IIC_Send_Byte(0XA0);	   
	M24512_IIC_Wait_Ack();
	M24512_IIC_Send_Byte((u8)((addr>>8)&0xFF));
	M24512_IIC_Wait_Ack();		 
  M24512_IIC_Send_Byte((u8)(addr&0XFF));   
	M24512_IIC_Wait_Ack();	
	
	M24512_IIC_Start();  	 	   
	M24512_IIC_Send_Byte(0XA1);       		   
	M24512_IIC_Wait_Ack();	 
  temp=M24512_IIC_Read_Byte(1);
	temp=(temp<<8)|M24512_IIC_Read_Byte(0);	
  M24512_IIC_Stop(); 
	return temp;
}

/************************* M24512д������������ ***********************/
void M24512_WriteLen(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
	u8 i;
	M24512_IIC_Start();  
	M24512_IIC_Send_Byte(0XA0);	 
	M24512_IIC_Wait_Ack();
	M24512_IIC_Send_Byte(WriteAddr>>8);
	M24512_IIC_Wait_Ack();	   
  M24512_IIC_Send_Byte(WriteAddr); 
	M24512_IIC_Wait_Ack(); 
	for(i=0;i<NumToWrite;i++)
	{
		M24512_IIC_Send_Byte((u8)(pBuffer[i]>>8)); 
		M24512_IIC_Wait_Ack(); 	 										  		   
		M24512_IIC_Send_Byte((u8)(pBuffer[i])); 
		M24512_IIC_Wait_Ack();	
	}
  M24512_IIC_Stop();
	delay_ms(10);
	
	if((WriteAddr+NumToWrite*2)%10==0)	M24512_WriteWord((WriteAddr+NumToWrite*2),pBuffer[i-1]);
}

/*********************** M24512��ȡ���������� ***************************/
void M24512_ReadLen(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
	u16 i;
  M24512_IIC_Start();  
	M24512_IIC_Send_Byte(0XA0);	   
	M24512_IIC_Wait_Ack();
	M24512_IIC_Send_Byte((u8)((ReadAddr>>8)&0xFF));
	M24512_IIC_Wait_Ack();		 
  M24512_IIC_Send_Byte((u8)(ReadAddr&0XFF));   
	M24512_IIC_Wait_Ack();	
	
	M24512_IIC_Start();  	 	   
	M24512_IIC_Send_Byte(0XA1);       		   
	M24512_IIC_Wait_Ack();
	for(i=0;i<(NumToRead-1);i++)
	{
		pBuffer[i]=M24512_IIC_Read_Byte(1);
		pBuffer[i]=(pBuffer[i]<<8)|M24512_IIC_Read_Byte(1);	
	}
	pBuffer[NumToRead-1]=M24512_IIC_Read_Byte(1);
	pBuffer[NumToRead-1]=(pBuffer[i]<<8)|M24512_IIC_Read_Byte(0);
  M24512_IIC_Stop(); 
}

/**************** M24512�Լ���� **************************/
u8 M24512_Check()
{
	u16 temp;
	temp=M24512_ReadWord(100);	   
	if(temp==0x55AA)return 0;		   
	else
	{
		M24512_WriteWord(100,0x55AA);
		temp=M24512_ReadWord(100);
		if(temp==0x55AA)return 0;
	}
	return 1;
}
