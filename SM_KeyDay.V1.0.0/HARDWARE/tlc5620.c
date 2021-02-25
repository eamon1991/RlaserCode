#include "sys.h"

#define RV				2.0			 //参考电压2V			
#define Data_1()	(PBout(15)=1)
#define Data_0()	(PBout(15)=0)	
#define Load_1()	(PBout(14)=1)
#define Load_0()	(PBout(14)=0)	
#define Clk_1()		(PCout(6)=1)
#define Clk_0()		(PCout(6)=0)

static void TLC5620SendBit(unsigned char dat)
{
	if(dat == 0)	{Data_0();}
	else	{Data_1();}
	Clk_1();
	Clk_0();
}

static void TLC5620SendByte(unsigned char dat)
{
	u8 i;

	for(i=0;i<8;i++)
	{
		TLC5620SendBit(dat & 0x80);
		dat	<<= 1;
	}
}

static void TLC5620SendAddr(unsigned char addr)
{
	u8 i;
	addr <<= 6;

	for(i=0;i<2;i++)
	{
		TLC5620SendBit(addr & 0x80);
		addr	<<=	1;
	}
	TLC5620SendBit(1);
}


void TLC5620_Set_DA(u8 Addr,float volt) //设定ADDR通道 输出电压VOLT
{
	u8 dat = (u8)((volt/2.0/RV)*256-1);
	Clk_0();
	TLC5620SendAddr(Addr);
	TLC5620SendByte(dat);
	Load_0();
	Load_1();
}

void tlc5620_init()
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE );	
	
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_14|	GPIO_Pin_15;	// load | data
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_6;	//clk
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}
