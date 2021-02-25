#include "sys.h"

#define baud 115200

static u8 Rx2_Buf[255];
static u8 Tx2_Buf[255]={0xF7,0x00,0x000,0x00,0x00,0x00};

static frmFmt frmFmt_2;
static enum cmdStatus  cmdStatus_2;
static u8 receive_flag2;

void usart2_init()
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
	DMA_InitStructure.DMA_MemoryBaseAddr	=	(uint32_t)Tx2_Buf;	
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
	DMA_InitStructure.DMA_MemoryBaseAddr	=	(uint32_t)Rx2_Buf;
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
	
	USART_InitStructure.USART_BaudRate  = baud;
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

void USART2_SendByte(u8 dat)
{
	USART_SendData(USART2,dat);
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)    ==  RESET);
}

void DMA_usart2_Send()
{
	DMA_Cmd(DMA1_Channel4,DISABLE);
	DMA1_Channel4->CMAR	=	(uint32_t)Tx2_Buf;
	DMA1_Channel4->CNDTR	=	Tx2_Buf[6]+8;
	DMA_Cmd(DMA1_Channel4, ENABLE);
	while(DMA_GetITStatus(DMA1_FLAG_TC4)==RESET);
	DMA_ClearFlag(DMA1_FLAG_TC4);
	DMA_ClearITPendingBit(DMA1_IT_GL4);
	DMA_Cmd(DMA1_Channel4,DISABLE);
}

static void Parameters2(u8 dat)  //解析函数
{
  static u8 prev;
  static u8 temp;  

  prev = dat;

  if((prev == 0xF7)&(cmdStatus_2 == stateIdle))
  {
    cmdStatus_2 = stateStart;
  }

  switch(cmdStatus_2)  //枚举类型 状态机参数
  {
    case  stateStart:  frmFmt_2.flag = 0xf7;
                       temp = 0;
                       cmdStatus_2 = stateDest;
                       break;

    case  stateDest:  frmFmt_2.DestAddr = (frmFmt_2.DestAddr<<8)|prev;
                      temp++;
                      if(temp == 2)
                      {
                        temp = 0;
                        cmdStatus_2 = stateSrc;
                      }
                      break;
                      
    case  stateSrc: frmFmt_2.SrcAddr  = (frmFmt_2.SrcAddr<<8)|prev;
                    temp++;
                    if(temp == 2)
                    {
                      temp  = 0;
                      cmdStatus_2 = stateType; 
                    }
                    break;
                    
    case  stateType:  frmFmt_2.Type = prev;
                      cmdStatus_2 = stateLength;
                      break;
                      
    case  stateLength:  frmFmt_2.Length = prev;
                        cmdStatus_2 = stateData;
                        break;
                        
    case  stateData:  frmFmt_2.buf[temp] =  prev;
                      temp++;
                      if(temp ==  frmFmt_2.Length)
                      {
                        temp  = 0;
                        cmdStatus_2 = stateVerify;
                      }
                      break;
                      
    case  stateVerify:  frmFmt_2.verify = prev;
                        cmdStatus_2 = stateIdle;
                        receive_flag2  = 1;
                        break;
                        
    default:  break;
  }
  
}

#if 0
void USART2_IRQHandler(void)
{
	u8 temp,i;
	
	if(USART_GetITStatus(USART1,USART_IT_IDLE) != RESET)
	{
		temp = USART1->SR;  
    temp = USART1->DR; 
		
		DMA_Cmd(DMA1_Channel5,DISABLE);	
		DMA_ClearFlag(DMA1_FLAG_GL5);
		
		temp	=	60	-	DMA_GetCurrDataCounter(DMA1_Channel5);	
		
		for(i=0; i< temp; i++)	{Parameters2(Rx2_Buf[i]);}
		
		DMA1_Channel5->CNDTR	=	60;	
		DMA_Cmd(DMA1_Channel5,ENABLE);
	}
}
#endif

void com2_app()	//处理云端 接收到的数据
{
	if(receive_flag2 == 1)
	{
		receive_flag2 = 0;
		switch((frmFmt_2.buf[0]<<8)|frmFmt_2.buf[1])
		{
			case 0x0101:break;
			
			case 0x0102:break;
			
			default: break;
		}
	}
}
