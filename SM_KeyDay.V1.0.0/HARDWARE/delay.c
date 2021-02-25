#include "sys.h"

static	u8 	fac_us	=	0;
static	u16 fac_ms	=	0;

void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	
	fac_us	=	SystemCoreClock/8000000;
	fac_ms	=	fac_us	*	1000;
}

void delay_us(u32 n)
{
	u32	temp;
	SysTick->LOAD	=	n*fac_us;
	SysTick->VAL	=	0;
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;        
	do
	{
		temp	=	SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));
	SysTick->CTRL	&=	~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL	=	0;
}

void delay_ms(u32 n)
{
	u32 temp;		   
	SysTick->LOAD=(u32)n*fac_ms;	
	SysTick->VAL =0x00;           
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;   
	SysTick->VAL =0X00;   
}
