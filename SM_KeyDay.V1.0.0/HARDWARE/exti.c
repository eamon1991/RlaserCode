#include "sys.h"

extern u8 PD1_Err_Flag;
extern u8 PD2_Err_Flag;

void exti_init()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource4);	//Module_1 	
	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure);	  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								
	NVIC_Init(&NVIC_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource3);	//Module _2	关光
	EXTI_InitStructure.EXTI_Line=EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure);	  	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								
	NVIC_Init(&NVIC_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource5);	
	EXTI_InitStructure.EXTI_Line=EXTI_Line5;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure);	  	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								
	NVIC_Init(&NVIC_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource2);	//Module_1 	
	EXTI_InitStructure.EXTI_Line=EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure);	  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								
	NVIC_Init(&NVIC_InitStructure);
}

u8 falling_flag = 0; //下降沿标志
extern u8 err_flag;
void EXTI3_IRQHandler(void)	  //根据输入PWM同步输出PWM
{
	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
			if(info_1.lockflag == 0)
			{
				Module_Off;
				Run_LED_Off; // 测试 LED测试 外控 信号；
				TIM3->CNT = 0;
				falling_flag = 1;
				TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
			}
		EXTI_ClearITPendingBit(EXTI_Line3);	
	}	
}
void EXTI4_IRQHandler(void) //根据输入PWM同步输出PWM  出光
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
			if(info_1.lockflag == 0)
			{
				Set_PA11_IO();
				if(err_flag==0) //无报警
				{
				WorkMode = ExtMode;
				Module_On;
				if(info_1.red_enable&&RED_ENABLE_IN) Red_guid_Off;	//加入红光使能
				Run_LED_On;
				falling_flag = 0;
				}
				else Module_Off;//关光
				TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
			}
		EXTI_ClearITPendingBit(EXTI_Line4);	
	}	
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
//		if(WorkMode == InMode)	
//		{
//			Set_PA11_PWM();
//			WorkMode = InMode;
//			Set_PWM(10,0);
//			info_1.power = 0;
//			Set_LD_Volt(info_1.power);
//		}
//		else
//		{
//			Set_PA11_IO();
//			WorkMode = ExtMode;
//			if(info_1.power_gather >=100)	info_1.power = 100;
//			else info_1.power = info_1.power_gather;
//			Set_LD_Volt(info_1.power);
//		}
		EXTI_ClearITPendingBit(EXTI_Line5);	
	}	
}

void EXTI2_IRQHandler(void)	  //根据输入PWM同步输出PWM
{
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
			if(RED_ENABLE_IN == 1)
			{
				Red_guid_On;
			}
			else
			{
				Red_guid_Off;
			}
		EXTI_ClearITPendingBit(EXTI_Line2);	
	}	
}
