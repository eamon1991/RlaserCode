#include "sys.h"
#include "stdlib.h"

#define baudrate	115200

u8 Tx_Buf[255]={0xF7,0x00,0x000,0x00,0x00,0x00}; 	//���ڷ��ʹ洢��
static u8 Rx_Buf[255];														//���ڽ��մ洢��
Change_external Change_external_1; 								//�ͻ��ⲿ�ɱ����
extern u8 receive_flag;   												//����һ֡���ݽ���״̬ 0û��� 1���
static enum cmdStatus  cmdStatus_1; 							//ö������ ״̬������
extern frmFmt frmFmt_1; 													//�������ݽ�����


void usart1_init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;
	DMA_InitTypeDef	DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1, ENABLE );	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);	
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
	
  NVIC_InitStructure.NVIC_IRQChannel  = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA1_Channel4,DISABLE);	
	DMA_DeInit(DMA1_Channel4);	
	DMA_InitStructure.DMA_PeripheralBaseAddr	=	(uint32_t)(&USART1->DR);	
	DMA_InitStructure.DMA_MemoryBaseAddr	=	(uint32_t)Tx_Buf;	
	DMA_InitStructure.DMA_DIR	=	DMA_DIR_PeripheralDST;	
	DMA_InitStructure.DMA_BufferSize	=	60;	
	DMA_InitStructure.DMA_PeripheralInc	=	DMA_PeripheralInc_Disable;	
	DMA_InitStructure.DMA_MemoryInc	=	DMA_MemoryInc_Enable;	
	DMA_InitStructure.DMA_PeripheralDataSize	=	DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize	=	DMA_MemoryDataSize_Byte;	
	DMA_InitStructure.DMA_Mode	=	DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority	=	DMA_Priority_VeryHigh;	
	DMA_InitStructure.DMA_M2M	=	DMA_M2M_Disable;	
	DMA_Init(DMA1_Channel4,&DMA_InitStructure);	
	DMA_ClearFlag(DMA1_FLAG_GL4);	
	DMA_Cmd(DMA1_Channel4,DISABLE);
	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);
	
	DMA_Cmd(DMA1_Channel5,DISABLE);	
	DMA_DeInit(DMA1_Channel5);	
	DMA_InitStructure.DMA_PeripheralBaseAddr	=	(uint32_t)(&USART1->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr	=	(uint32_t)Rx_Buf;
	DMA_InitStructure.DMA_DIR	=	DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize	=	60;
	DMA_InitStructure.DMA_PeripheralInc	=	DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc	=	DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize	=	DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize	=	DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode	=	DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority	=	DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M	=	DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5,&DMA_InitStructure);
	DMA_ClearFlag(DMA1_FLAG_GL5);
	DMA_Cmd(DMA1_Channel5,ENABLE);
	
	USART_InitStructure.USART_BaudRate  = baudrate;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_WordLength  = USART_WordLength_8b;
  USART_InitStructure.USART_Parity  = USART_Parity_No;
  USART_InitStructure.USART_StopBits  = USART_StopBits_1;
	USART_InitStructure.USART_Mode  = USART_Mode_Rx|USART_Mode_Tx;
  USART_Init(USART1,&USART_InitStructure);
  USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);
  USART_Cmd(USART1,ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

void USART1_SendByte(u8 dat)
{
	USART_SendData(USART1,dat);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)    ==  RESET);
}

void DMA_usart1_Send()
{
	DMA_Cmd(DMA1_Channel4,DISABLE);
	DMA1_Channel4->CMAR	=	(uint32_t)Tx_Buf;
	DMA1_Channel4->CNDTR	=	Tx_Buf[6]+8;
	DMA_Cmd(DMA1_Channel4, ENABLE);
	while(DMA_GetITStatus(DMA1_FLAG_TC4)==RESET);
	DMA_ClearFlag(DMA1_FLAG_TC4);
	DMA_ClearITPendingBit(DMA1_IT_GL4);
	DMA_Cmd(DMA1_Channel4,DISABLE);
}

int fputc(int ch,FILE *f)
{
    USART_SendData(USART1,ch);

    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);

    return (ch);
}

void Parameters(u8 dat)  //��������
{
  static u8 prev;
  static u8 temp;  

  prev = dat;

  if((prev == 0xf7)&(cmdStatus_1 == stateIdle))
  {
    cmdStatus_1 = stateStart;
  }

  switch(cmdStatus_1)  //ö������ ״̬������
  {
    case  stateStart:  frmFmt_1.flag = 0xF7;
                       temp = 0;
                       cmdStatus_1 = stateDest;
                       break;

    case  stateDest:  frmFmt_1.DestAddr = (frmFmt_1.DestAddr<<8)|prev;
                      temp++;
                      if(temp == 2)
                      {
                        temp = 0;
                        cmdStatus_1 = stateSrc;
                      }
                      break;
                      
    case  stateSrc: frmFmt_1.SrcAddr  = (frmFmt_1.SrcAddr<<8)|prev;
                    temp++;
                    if(temp == 2)
                    {
                      temp  = 0;
                      cmdStatus_1 = stateType; 
                    }
                    break;
                    
    case  stateType:  frmFmt_1.Type = prev;
                      cmdStatus_1 = stateLength;
                      break;
                      
    case  stateLength:  frmFmt_1.Length = prev;
                        cmdStatus_1 = stateData;
                        break;
                        
    case  stateData:  frmFmt_1.buf[temp] =  prev;
                      temp++;
                      if(temp ==  frmFmt_1.Length)
                      {
                        temp  = 0;
                        cmdStatus_1 = stateVerify;
                      }
                      break;
                      
    case  stateVerify:  frmFmt_1.verify = prev;
                        cmdStatus_1 = stateIdle;
                        receive_flag  = 1;
                        break;
                        
    default:  break;
  }
  
}

void USART1_IRQHandler(void)
{
	u8 temp,i;
	
	if(USART_GetITStatus(USART1,USART_IT_IDLE) != RESET)
	{
		temp = USART1->SR;  
    temp = USART1->DR; 
		
		DMA_Cmd(DMA1_Channel5,DISABLE);	
		DMA_ClearFlag(DMA1_FLAG_GL5);
		
		temp	=	60	-	DMA_GetCurrDataCounter(DMA1_Channel5);	
		
		for(i=0; i< temp; i++)	{Parameters(Rx_Buf[i]);}
		
		DMA1_Channel5->CNDTR	=	60;	
		DMA_Cmd(DMA1_Channel5,ENABLE);
	}
}

u8 get_crc(u8 *dat,u8 len)  //CRCУ��
{
	u8 i,j;
	u8 crc = 0;
	for(j=0;j<len;j++)
	{
		crc ^= dat[j];
		for(i=0;i<8;i++)
		{
			if(crc&0x01)	crc=(u8)((crc>>1)^0x51);
			else crc>>=1;
		}
	}
	return crc;
}

void Send_WorkMode_Err()  //����
{
	Tx_Buf[6] = 2;
	Tx_Buf[7] = 0x01;
	Tx_Buf[8] = 0x01;
	Tx_Buf[9] = get_crc(Tx_Buf,(Tx_Buf[6]+7));
	DMA_usart1_Send();
}

void Send_Para()  //���Ͳ��� 
{
	Tx_Buf[6] = 6;
	Tx_Buf[7] = 0x01;
	Tx_Buf[8] = 0x02;
	Tx_Buf[9] = info_1.Para.red_set;  //�������
	Tx_Buf[10] = info_1.Para.pd1_set; //PD1 ��ѹ
	Tx_Buf[11] = info_1.Para.pd2_set; //PD2 ��ѹ
	Tx_Buf[12] = info_1.Para.Tover_set; //�¶ȱ�����ֵ
	Tx_Buf[13] = get_crc(Tx_Buf,(u8)(Tx_Buf[6]+7)); 
	DMA_usart1_Send();
}


void Send_Screen() //���ͱ���������
{
	Tx_Buf[6] = 4;
	Tx_Buf[7] = 0x01;
	Tx_Buf[8] = 0x03;
	Tx_Buf[9] = info_1.Para.alarm_screen>>8;
	Tx_Buf[10] = info_1.Para.alarm_screen;
	Tx_Buf[11] = get_crc(Tx_Buf,(Tx_Buf[6]+7));
	DMA_usart1_Send();
}

void Send_Err(u8 err_code) //���ͱ�����Ϣ ���� err_code��������
{
	Tx_Buf[6] = 3;
	Tx_Buf[7] = 0x01;
	Tx_Buf[8] = 0x04;
	Tx_Buf[9] = err_code;
	Tx_Buf[10] = get_crc(Tx_Buf,(Tx_Buf[6]+7));
	DMA_usart1_Send();
}

static void ResponseLaserOn()
{
	Tx_Buf[6] = 2;
	Tx_Buf[7] = 0x0A;
	Tx_Buf[8] = 0x01;
	Tx_Buf[9] = get_crc(Tx_Buf,(Tx_Buf[6]+7));
	DMA_usart1_Send();
}

static void ResponseLaserOff()
{
	Tx_Buf[6] = 2;
	Tx_Buf[7] = 0x0A;
	Tx_Buf[8] = 0x00;
	Tx_Buf[9] = get_crc(Tx_Buf,(Tx_Buf[6]+7));
	DMA_usart1_Send();
}


extern u8 drive_num; //���������
extern u16 Average_I[16]; //0.5s 50��ƽ������
void Send_Status()  //�����豸״̬
{
	u8 i;
	u16 master_alarm_screen;
//	float *p = &info_1.current1;
	
	RTC_Get();	RTC_WaitForSynchro();
	
	Tx_Buf[6] = 46+drive_num*2;
	
	//����
	Tx_Buf[7] = 0x01;	
	Tx_Buf[8] = 0x06;
	
	//����������	
	Tx_Buf[9] = info_1.Para.red_set;		
	Tx_Buf[10] = info_1.Para.pd1_set;
	Tx_Buf[11] = info_1.Para.pd2_set;
	Tx_Buf[12] =info_1.Para.Tover_set; //�¶���ֵ
	Tx_Buf[13] = (info_1.Para.Iover_set)&0xFF; //������ֵ
	Tx_Buf[14] = (info_1.Para.Imax_set)&0xFF;  //������ֵ
	
	//����������
	master_alarm_screen=info_1.Para.alarm_screen;
	#if 0
	if(Change_external_1.Flow_enable==1) master_alarm_screen=master_alarm_screen|0X2000;
	else master_alarm_screen=master_alarm_screen&(~0X2000);
	if(Change_external_1.QBH_enable==1) master_alarm_screen=master_alarm_screen|0X0004;
	else master_alarm_screen=master_alarm_screen&(~0X0004);
	#endif
	Tx_Buf[15] = master_alarm_screen>>8; //����������
	Tx_Buf[16] = master_alarm_screen; 
	
	//������Ϣ
	Tx_Buf[17] = info_1.lockflag;	 //������־
	Tx_Buf[18] = info_1.locktype;	 //����״̬
	Tx_Buf[19] = info_1.locknum;  //��������
	Tx_Buf[20] = info_1.lockyear>>8;  //����ʱ��
	Tx_Buf[21] = info_1.lockyear;
	Tx_Buf[22] = info_1.lockmonth;
	Tx_Buf[23] = info_1.lockday;
	Tx_Buf[24] = info_1.lockhour;
	Tx_Buf[25] = info_1.lockmin;
	
	//�豸�ڲ�ʱ��
	Tx_Buf[26] = calendar.year>>8;
	Tx_Buf[27] = calendar.year;
	Tx_Buf[28] = calendar.month;
	Tx_Buf[29] = calendar.day;
	Tx_Buf[30] = calendar.hour;
	Tx_Buf[31] = calendar.min;
	Tx_Buf[32] = calendar.sec;
	
	//������Ϣ
	Tx_Buf[33] = info_1.power; //���빦��
	
	//4·�¶���Ϣ
	Tx_Buf[34] = info_1.temp1; //�¶�1
	Tx_Buf[35] = info_1.temp2; //�¶�2
	Tx_Buf[36] = info_1.temp3; //�¶�3
	Tx_Buf[37] = info_1.temp4;  //�¶�4
	
	//���������
	Tx_Buf[38] = drive_num;
	
	//���к�
	Tx_Buf[39] = info_1.serialnum1; //���к�
	Tx_Buf[40] = info_1.serialnum2; //���к�
	Tx_Buf[41] = info_1.serialnum3; //���к�
	Tx_Buf[42] = info_1.serialnum4;  //���к�
	Tx_Buf[43] = (info_1.serialnum5>>8)&0xFF;
	Tx_Buf[44] = info_1.serialnum5;  //���к� 38
	
	//��¼��Ϣ�Լ�����汾��Ϣ
	Tx_Buf[45] = info_1.key_time;  //��¼���� 39
	Tx_Buf[46]=(info_1.Code_Editions>>8)&0XFF; //����汾 40   v3.00
	Tx_Buf[47]=(u8)info_1.Code_Editions; //����汾 40
	Tx_Buf[48]=0;  //Ԥ��PD ��ʾ����
	Tx_Buf[49]=0;
	Tx_Buf[50]=0;
	Tx_Buf[51]=(info_1.Para.Iover_set>>8)&0xFF;;
	Tx_Buf[52]=(info_1.Para.Imax_set>>8)&0xFF;

	//������Ϣ
	//for(i=0;i<drive_num;i++)	Tx_Buf[45+i] = (u8)((*p++)*10); //����1-num 
	for(i=0;i<drive_num;i++) 
	{
		Tx_Buf[53+i] =	(Average_I[i]>>8)&0xFF; //0.5s 50��ƽ������
		Tx_Buf[54+i] = 	Average_I[i]&0xFF;
	}
	
	Tx_Buf[53+drive_num*2] = get_crc(Tx_Buf,(Tx_Buf[6]+7));
	
	
	DMA_usart1_Send();
}

void Send_ErrSave()
{
	errSaveInfo err[3];
	STMFLASH_Read(ErrInfo1Addr1,&err[0].flag,24);
	Tx_Buf[6] = 29;
	Tx_Buf[7] = 0x01;
	Tx_Buf[8] = 0x05;
	Tx_Buf[9] = err[0].flag;
	Tx_Buf[10] = err[0].year>>8;
	Tx_Buf[11] = err[0].year;
	Tx_Buf[12] = err[0].month;
	Tx_Buf[13] = err[0].day;
	Tx_Buf[14] = err[0].hour;
	Tx_Buf[15] = err[0].min;
	Tx_Buf[16] = err[0].sec;
	Tx_Buf[17] = err[0].errCode;
	Tx_Buf[18] = err[1].flag;
	Tx_Buf[19] = err[1].year>>8;
	Tx_Buf[20] = err[1].year;
	Tx_Buf[21] = err[1].month;
	Tx_Buf[22] = err[1].day;
	Tx_Buf[23] = err[1].hour;
	Tx_Buf[24] = err[1].min;
	Tx_Buf[25] = err[1].sec;
	Tx_Buf[26] = err[1].errCode;
	Tx_Buf[27] = err[2].flag;
	Tx_Buf[28] = err[2].year>>8;
	Tx_Buf[29] = err[2].year;
	Tx_Buf[30] = err[2].month;
	Tx_Buf[31] = err[2].day;
	Tx_Buf[32] = err[2].hour;
	Tx_Buf[33] = err[2].min;
	Tx_Buf[34] = err[2].sec;
	Tx_Buf[35] = err[2].errCode;
	Tx_Buf[36] = get_crc(Tx_Buf,(Tx_Buf[6]+7));
	DMA_usart1_Send();
}

//��¼����д��ڼ���
//��ʽ�������룬19�꣬01�£�10�գ�12ʱ��
void Write_Err_eeprom(u8 err_code) //������ MCU��¼
{
	u16 ERR[5];
	u16 History_sign;
	RTC_Get();							//ˢ��ʱ��
	RTC_WaitForSynchro();  	//�ȴ�ͬ��
	ERR[0]=err_code;				// ������
	ERR[1]=calendar.year;		// ��
	ERR[2]=calendar.month;	//get month
	ERR[3]=calendar.day;		//get day
	ERR[4]=calendar.hour; 	//ʱ
	History_sign=Read_Flash_Word(Err_History_sign);// ��ȡ��¼�ĵڼ���
	if(History_sign>=20)
	{
		Write_Flash_Word(Err_History_sign,0);
		History_sign=0;
	}
  STMFLASH_Write(Err_History(History_sign),(u16*)ERR,5); //FLASHд������

	History_sign++;
	Write_Flash_Word(Err_History_sign,History_sign);			 //��¼�ĵڼ���
	
}

void clean_Err_eeprom(void) //������ MCU��¼���
{
	u16 ERR[5]={0XFFFF,0XFFFF,0XFFFF,0XFFFF,0XFFFF};
	u8 History_sign;
		Write_Flash_Word(Err_History_sign,0);
	for(History_sign=0;History_sign<20;History_sign++)
	{
  STMFLASH_Write(Err_History(History_sign),(u16*)ERR,5); //FLASHд������
	}	
}

//��ʽ�������룬19�꣬01�£�10�գ�12ʱ
//�洢λ�� 0-19 20�鱨����ʷ  ������==0X00 ���ͱ���ʹ��ʱ��
void Send_PC_Err_History(u8 addr) //���ʹ洢 ��ʷ��������
{
	u16 ERR[5];
	u16 History_sign;
	u16 Use_time_L,Use_time_H; // ������ʹ��ʱ�� min month
	if(addr<20) //�����ź� 0-19
	{
		History_sign=Read_Flash_Word(Err_History_sign);// ��ȡ��¼�ĵڼ���
		History_sign=History_sign+addr;
		if(History_sign>19)
		{
			History_sign=History_sign-20;
		}
		if(History_sign<20)
		{
			STMFLASH_Read(Err_History(History_sign),(u16*)ERR,5);	//FLASH��ȡ����
			Tx_Buf[6] = 0x07;
			Tx_Buf[7] = 0x01;  								//������
			Tx_Buf[8] = 0x07;  								//������
			Tx_Buf[9] = (u8)ERR[0];   				//�����־
			Tx_Buf[10] = (u8)(ERR[1]-2000);   //��
			Tx_Buf[11] = (u8)ERR[2];   				//��
			Tx_Buf[12] = (u8)ERR[3];   				//��
			Tx_Buf[13] = (u8)ERR[4];   				//ʱ
			Tx_Buf[14] = get_crc(Tx_Buf,(Tx_Buf[6]+7));
			if((Tx_Buf[10]<100)&(Tx_Buf[11]<=12)&(Tx_Buf[12]<=31)&(Tx_Buf[13]<=24)) //������ȷ
			{
				DMA_usart1_Send();
			}
		}
  }
	else
	{
		Use_time_L=Read_Flash_Word(Use_time_min);//��ȡ����ʹ��ʱ�� min 30*24*60=43200
		Use_time_H=Read_Flash_Word(Use_time_month);//��ȡ����ʹ��ʱ�� month
		
		Tx_Buf[6] = 0x07;
	  Tx_Buf[7] = 0x01;  //������
	  Tx_Buf[8] = 0x07;  //������
	  Tx_Buf[9] = 0X00;   //�����־=��
	  Tx_Buf[10] = (u8)(Use_time_H>>8);   //month_H ����
		Tx_Buf[11] = (u8)(Use_time_H);   //month_L
		Tx_Buf[12] = (u8)(Use_time_L>>8);   //min_H
		Tx_Buf[13] = (u8)(Use_time_L);   //min_L
	  Tx_Buf[14] = get_crc(Tx_Buf,(Tx_Buf[6]+7));  
	  DMA_usart1_Send();	
	}
}

//��ȡ������ 
u8 Read_Err_History(u8 addr) //���addr ��ȡ���汨��
{
	s16 History_sign;
	u8 Error=0;
	History_sign=Read_Flash_Word(Err_History_sign);// ��ȡ��¼�ĵڼ���
	if(History_sign<=20)
	{
		
	 History_sign=History_sign-addr-1;
	 if(History_sign<0)
	 {
		History_sign=History_sign+20;	
	 }	
	 if((History_sign>=0)&&(History_sign<20))
	  Error = Read_Flash_Word(Err_History(History_sign));   //�����־
  }
	else Error = 0;
	return Error;	
}


u8 Err_addr=100; 						//��ʷ���� ��ȡ����
u8 internal_laser_flag = 0;
extern u8 falling_flag; 		//�½��ر�־

void com_app()	//������λ�� ���յ�������
{
	u16 Set_Cmd1,Set_Cmd2;
  static u8 Login_key=1; //ÿ�ο�����¼һ��
	if(receive_flag == 1)
	{
		receive_flag = 0;
		switch((frmFmt_1.buf[0]<<8)|frmFmt_1.buf[1])
		{
			case IntPowerCmd:	  //�ڿس�������	0101
				if(info_1.lockflag == 0)	
				{
					if(frmFmt_1.buf[2] == 1)
					{if(WorkMode == InMode)
						{	
							if((frmFmt_1.buf[3]<=100)&&(frmFmt_1.buf[5]<=100)) //
							{
								if(info_1.test_or_rule==1) //������
								{if(frmFmt_1.buf[3]>5) //���ʴ���5%
									{
										TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
							      if(info_1.red_enable&&RED_ENABLE_IN)Red_guid_Off;Run_LED_On;
							      info_1.power = frmFmt_1.buf[3];
										if(info_1.power<10) info_1.power_change=10;
										 else info_1.power_change=info_1.power;
							      Set_LD_Volt(info_1.power_change);
							      delay_us(5);
								    falling_flag = 0; //����PD����ֵ
							      Set_PWM(frmFmt_1.buf[4],frmFmt_1.buf[5]); internal_laser_flag = 1;
										ResponseLaserOn();
									}
								}
								else //������
								{
							  TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
							  if(info_1.red_enable&&RED_ENABLE_IN)Red_guid_Off;Run_LED_On;
							  info_1.power = frmFmt_1.buf[3];
							  Set_LD_Volt(info_1.power);
							  delay_us(5);
								falling_flag = 0; //����PD����ֵ
							  Set_PWM(frmFmt_1.buf[4],frmFmt_1.buf[5]); internal_laser_flag = 1;
								ResponseLaserOff();
								}
							}
						}
					}else{info_1.power = 0; falling_flag = 1;Set_LD_Volt(0);Set_PWM(10,0);if(info_1.red_enable&&RED_ENABLE_IN)Red_guid_On;Run_LED_Off;internal_laser_flag =0;}
				}
				break;
			case SetRedCmd: //�趨������� 0102
				info_1.Para.red_set = frmFmt_1.buf[2];
				Write_Flash_Word(ParaRedAddr,info_1.Para.red_set);
				TLC5620_Set_DA(RED_Channel,(float)(info_1.Para.red_set*4/1000.0));
				break;
			case SetPd1Cmd:	 //�趨PD1 0103
		  	info_1.Para.pd1_set = frmFmt_1.buf[2];
				Write_Flash_Word(ParaPD1Addr,info_1.Para.pd1_set);
				TLC5620_Set_DA(PD1_Channel,(float)(info_1.Para.pd1_set/10.0));
				break;
			case SetPd2Cmd:	 //�趨PD2  0104
				info_1.Para.pd2_set = frmFmt_1.buf[2];
				Write_Flash_Word(ParaPD2Addr,info_1.Para.pd2_set);
				TLC5620_Set_DA(PD2_Channel,(float)(info_1.Para.pd2_set/10.0));
				break;
			case SetToverCmd:	  //�趨�¶ȱ���  0105
				info_1.Para.Tover_set = frmFmt_1.buf[2];
				Write_Flash_Word(ParaToverAddr,info_1.Para.Tover_set);
				break;
			case SetIoverCmd:	 //�趨��������  0106
				info_1.Para.Iover_set = (u16)(frmFmt_1.buf[2]<<8)|(frmFmt_1.buf[3]);
				Write_Flash_Word(ParaIoverAddr,info_1.Para.Iover_set);
				break;
			case SetImaxCmd:	 //�趨������ֵ 0107
				info_1.Para.Imax_set = (u16)(frmFmt_1.buf[2]<<8)|(frmFmt_1.buf[3]);
				Write_Flash_Word(ParaImaxAddr,info_1.Para.Imax_set);
				break;				
			case SetScrenCmd:	 //���α���   0108 
				Set_Cmd1=info_1.Para.alarm_screen&(~0X2004);
			  Set_Cmd2=((frmFmt_1.buf[2]<<8)|frmFmt_1.buf[3])&(~0X2004);
			  if(Set_Cmd1!=Set_Cmd2) //��������
				{
				info_1.Para.alarm_screen = (frmFmt_1.buf[2]<<8)|frmFmt_1.buf[3];
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				}	
				Set_Cmd1=((frmFmt_1.buf[2]<<8)|frmFmt_1.buf[3])&0X2000; //ˮѹ����
				Set_Cmd2=((frmFmt_1.buf[2]<<8)|frmFmt_1.buf[3])&0X0004; //QBH����
				if(Set_Cmd1==0X2000) {Change_external_1.Flow_enable=1;Write_Flash_Word(ADDR4_Flow,1);}
				else {Change_external_1.Flow_enable=0;Write_Flash_Word(ADDR4_Flow,0);}
				if(Set_Cmd2==0X0004) {Change_external_1.QBH_enable=1; Write_Flash_Word(ADDR4_QBH,1);}
				else {Change_external_1.QBH_enable=0;	 Write_Flash_Word(ADDR4_QBH,0);}
				break;
				
			case SetTimeCmd:	  //�趨�������ڲ�ʱ�� ���ư�ʱ������λ��ͬ�� 010A
				RTC_EnterConfigMode();
				RTC_Set((frmFmt_1.buf[2]<<8|frmFmt_1.buf[3]),frmFmt_1.buf[4],frmFmt_1.buf[5],frmFmt_1.buf[6],frmFmt_1.buf[7],frmFmt_1.buf[8]);
				RTC_ExitConfigMode();
				break;
			case ReadInfoCmd:	   //��ȡ�豸���� 010B
				Send_Status();break;
			case ReadErrCmd:	  //��ȡ���󱣴���Ϣ����   010C
				Send_ErrSave();break;		
			case UnclockCmd:	  //��������    0109  ��0F09
				if(frmFmt_1.buf[2]!=7) //���
				{
				Write_Flash_Word(LockFlagAddr, 0);
				info_1.lockflag = Read_Flash_Word(LockFlagAddr);
				RTC_Get();
				RTC_WaitForLastTask();
				RTC_WaitForSynchro();
				RTC_ITConfig(RTC_IT_ALR	,ENABLE);
				RTC_WaitForLastTask();
				RTC_EnterConfigMode();
			  if(frmFmt_1.buf[2]<7) 
					{
				switch(frmFmt_1.buf[2])  ////����ʱ���趨 RTC_GetCounter()����ֵ //RTC�趨���� ���������ж�
				{
					case 0:RTC_SetAlarm(RTC_GetCounter()+OneMonth  );break;	
					case 1:RTC_SetAlarm(RTC_GetCounter()+TwoMonth  );break;
					case 2:RTC_SetAlarm(RTC_GetCounter()+ThreeMonth);break;	
					case 3:RTC_SetAlarm(RTC_GetCounter()+FourMonth );break;	
					case 4:RTC_SetAlarm(RTC_GetCounter()+FiveMonth );break;	
					case 5:RTC_SetAlarm(RTC_GetCounter()+SixMonth  );break;	
					case 6:RTC_ITConfig(RTC_IT_ALR,DISABLE);break; //�����ν��������ü���
				} }
				else // >8
					{
				    RTC_SetAlarm(RTC_GetCounter()+time_day(frmFmt_1.buf[2]-7));
					}
				RTC_WaitForLastTask();  //ʵʱʱ�� 
				RTC_ExitConfigMode();
				Write_Flash_Word(LockTypeAddr, (u16)(frmFmt_1.buf[2]));	//��¼����������ʱ���¼��Flash��
				Write_Flash_Word(LockNumAddr, (Read_Flash_Word(LockNumAddr)+1));
				Write_Flash_Word(LockYearAddr, calendar.year);   //ʵʱʱ��
				Write_Flash_Word(LockMonthAddr,calendar.month);
				Write_Flash_Word(LockDayAddr,  calendar.day);
				Write_Flash_Word(LockHourAddr, calendar.hour);
				Write_Flash_Word(LockMinAddr,  calendar.min);	
				info_1.locknum = (u8)Read_Flash_Word(LockNumAddr); //ʵʱʱ��
				info_1.locktype = (u8)Read_Flash_Word(LockTypeAddr);
				info_1.lockflag = (u8)Read_Flash_Word(LockFlagAddr);
				info_1.lockyear = Read_Flash_Word(LockYearAddr);
				info_1.lockmonth = (u8)Read_Flash_Word(LockMonthAddr);
				info_1.lockday = (u8)Read_Flash_Word(LockDayAddr);
				info_1.lockhour =(u8) Read_Flash_Word(LockHourAddr);
				info_1.lockmin = (u8)Read_Flash_Word(LockMinAddr);
				TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
				Ext_Module_Enable();
				Alarm_LED_Off;
				Run_LED_Off;
				Buzzer_Off;
				Write_Err_eeprom(VOID_flash);//������ʷд���
			}
				else //���������ʷ�͹�ʱ��ͼ��ܴ���
				{
					Write_Flash_Word(LockNumAddr, 0);//
					Write_Flash_Word(Use_time_min, 0);//������0
					Write_Flash_Word(Use_time_month,0);// ��ȡ����ʹ����		
          clean_Err_eeprom(); //������ MCU��¼���	
          info_1.locknum = (u8)Read_Flash_Word(LockNumAddr);
					Write_Flash_Word(usekey_times, 0);// //��¼����					
				}
				break;
				
			case WriteSpCmd:   // д�����к�  010E
				if(frmFmt_1.buf[2]!='A')
				{
					//if(info_1.serialnum1=='T'); // ��һ������SN
				Write_Flash_Word(SeriaNumAddr1, frmFmt_1.buf[2]);
				Write_Flash_Word(SeriaNumAddr2, frmFmt_1.buf[3]);
				Write_Flash_Word(SeriaNumAddr3, frmFmt_1.buf[4]);
				Write_Flash_Word(SeriaNumAddr4, frmFmt_1.buf[5]);
				Write_Flash_Word(SeriaNumAddr5, (u16)((frmFmt_1.buf[6]<<8)|frmFmt_1.buf[7]));
				info_1.serialnum1 = (u8)Read_Flash_Word(SeriaNumAddr1);
				info_1.serialnum2 = (u8)Read_Flash_Word(SeriaNumAddr2);
				info_1.serialnum3 = (u8)Read_Flash_Word(SeriaNumAddr3);
				info_1.serialnum4 = (u8)Read_Flash_Word(SeriaNumAddr4);
				info_1.serialnum5 = Read_Flash_Word(SeriaNumAddr5);
				}
				break;
			case 0x0201:   //QBH ����0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x01, 0x00(1=QBH_EN), 0x00
				if(frmFmt_1.buf[2]) {Change_external_1.QBH_enable=1; Write_Flash_Word(ADDR4_QBH,1);}
				else {Change_external_1.QBH_enable=0;	 Write_Flash_Word(ADDR4_QBH,0);}
				break;
			
			case 0x0202:	//������
				if(frmFmt_1.buf[2] == 1)
				{
					info_1.red_enable = 1;
					Write_Flash_Word(RedStatusAddr,info_1.red_enable);					
					if(WorkMode == InMode)	{if(internal_laser_flag == 1) Red_guid_Off; else Red_guid_On;}
					else if(WorkMode == ExtMode) {if(PBin(3)==0) Red_guid_On; else Red_guid_Off;}
				}else if(frmFmt_1.buf[2] == 0)
				{
					
					info_1.red_enable = 0;
					Write_Flash_Word(RedStatusAddr,info_1.red_enable);
					Red_guid_Off;
				}
				break;
			
			case 0x0203://�ڿع���ˢ��
				if(internal_laser_flag == 1)	{info_1.power = frmFmt_1.buf[2];Set_LD_Volt(info_1.power);}
				break;
				
			//case 0x0204:Write_Flash_Word(LockNumAddr, 0);info_1.locknum = Read_Flash_Word(LockNumAddr);break; //������ܴ���
			case 0x0205:lock_action();break; //���ں�����
				
			case 0x0206:Err_addr=0; //MCU�ڲ�������ʷ������
				break; //������ʷ��ȡ20��
				
			case 0x0207: File_Lock_up();  //�ļ����� ��������
			 break; //��������
				
			case 0x0208:  //��¼+1
				info_1.key_time = rand()%255;
				//random = rand()%255;
				Write_Flash_Word(usekey_times, (info_1.key_time));
				#if 0
				if(Login_key)
				{
					Write_Flash_Word(usekey_times, (info_1.key_time+1));// //��¼����
					Login_key=0;
				}
				#endif
				break;
				
			/******************* �������������� *************************/
			/******************** add by yym 19.9.21 ********************/
			case 0x0501:	//����PD1����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<0);
				else info_1.Para.alarm_screen &= ~(1<<0);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x0502:	//����PD2����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<1);
				else info_1.Para.alarm_screen &= ~(1<<1);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x0503:	//����QBH����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<2);
				else info_1.Para.alarm_screen &= ~(1<<2);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				File_Lock_up();
				break;
			case 0x0504:	//����T1����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<3);
				else info_1.Para.alarm_screen &= ~(1<<3);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x0505:	//����T2����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<4);
				else info_1.Para.alarm_screen &= ~(1<<4);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x0506:	//����T3����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<5);
				else info_1.Para.alarm_screen &= ~(1<<5);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x0507:	//����T4����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<6);
				else info_1.Para.alarm_screen &= ~(1<<6);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x0508:	//����I1����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<7);
				else info_1.Para.alarm_screen &= ~(1<<7);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x0509:	//����I2����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<8);
				else info_1.Para.alarm_screen &= ~(1<<8);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x050A:	//����I3����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<9);
				else info_1.Para.alarm_screen &= ~(1<<9);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x050B:	//����I4����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<10);
				else info_1.Para.alarm_screen &= ~(1<<10);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x500C:	//����I5����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<11);
				else info_1.Para.alarm_screen &= ~(1<<11);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x050D:	//����I6����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<12);
				else info_1.Para.alarm_screen &= ~(1<<12);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x050E: //����Watter����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= (1<<13);
				else info_1.Para.alarm_screen &= ~(1<<13);
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x050F: //����All����
				if(frmFmt_1.buf[2] == 1) info_1.Para.alarm_screen |= 0xFFFF;
				else info_1.Para.alarm_screen &= 0;
				Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
				break;
			case 0x0601:	//�����رպ��
				if(frmFmt_1.buf[1] == 1)	{	//����
					Red_guid_On;
				}else	{//�ر�
					Red_guid_Off;
				}
				break;
			/********************** end ************************************/	
			
			default:break;
		}		
	}
}

