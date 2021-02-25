#include "sys.h"

u16 USART2_RX_STA=0;       //接收状态标记	 
#define USART2_REC_LEN  			10  	//定义最大接收字节数 200
u8 USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
u8 USART2_Receive=0;//接收参数

extern u8 err_flag;  //错误标志


char PAGE[]="page jiemi\r\n"; //刷新界面0 （会闪屏）

char REST[]="rest\r\n";  //复位       （会闪屏）
char SN_WRITE[]="wristr 144,139,200,42,0,1055,65535,1,1,3,\"FFFFFFFFFF\"\r\n"; //SN写入
char SN_POWER[]="wristr 355,62,67,42,0,63488,65535,1,1,3,\"0000\"\r\n"; //功率写入  红色

char ALARM_WRITE[]="wristr 144,216,200,42,0,63488,65535,1,1,3,\"FF\"\r\n"; //报警代码 
char LOCK_day[]   ="wristr 275,216,100,42,0,62531,65535,0,1,3,\"000\"\r\n"; //剩余天数
 char PdCode1_2[]="page PdCode\r\n"; //PD报警
 char QbhCode3[]="page QbhCode\r\n"; //QBH报警
 char TCode4_7[]="page TCode\r\n"; //温度报警
 char CCode8_13[]="page CCode\r\n"; //电流报警
 char FwCode14[]="page FwCode\r\n"; //水压报警
 char StopCode15[]="page StopCode\r\n"; //急停报警
 char numberCode16[]="page nuberCode\r\n"; //参数改变
 char PD_continuity17[]="page PD_continuity\r\n"; //PD锁机
 char lock[]="page lock\r\n"; //加密到期
 
 
 char ALARM_WRITE00[]="wristr 144,216,200,42,1,1055,61309,1,1,3,\"正常\"\r\n"; //正常
 char ALARM_WRITE03[]="wristr 144,216,200,42,1,63488,61309,1,1,3,\"QBH报警\"\r\n"; //正常
 char ALARM_WRITE0E[]="wristr 144,216,200,42,1,63488,61309,1,1,3,\"水压报警\"\r\n"; //正常
 char ALARM_WRITE0F[]="wristr 144,216,200,42,1,63488,61309,1,1,3,\"急停报警\"\r\n"; //0x0F //急停报警
void USART2_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;   
    
    /* Enable the USART2 Pins Software Remapping */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB2Periph_AFIO, ENABLE); 
	  USART_DeInit(USART2);
    
	 /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;               //复用推完
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
     //Enable the USART2 Interrupt 
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);   
    
    USART_InitStructure.USART_BaudRate = 9600;//串口2波特率设置9600                
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//数据位数设置：8位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  //停止位设置：1位   
    USART_InitStructure.USART_Parity = USART_Parity_No;     //是否奇偶校验：无   
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流控制模式设置：没有使能
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  //接收与发送都使能  
  
    
    USART_Init(USART2, &USART_InitStructure); //初始化USART2
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);//Enable USART2
}

void uart2_send_byte(unsigned char byte)
{
// 将Printf内容发往串口 
  USART_SendData(USART2, byte);
  while (!(USART2->SR & USART_FLAG_TXE));
}



void UART2_PutStr (char *str)  
{  
    while (0 != *str)  
    {  
			uart2_send_byte(*str); 
        str++;  
    }  
}



void USART2_IRQHandler(void)  
{ 
		static u8 Res;//接收数据
	//******************防止串口死机*******************************************
	    if (USART_GetFlagStatus(USART2, USART_FLAG_PE) != RESET)
   {
       USART_ReceiveData(USART2);
     USART_ClearFlag(USART2, USART_FLAG_PE);
   }
    
   if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
   {
       USART_ReceiveData(USART2);
     USART_ClearFlag(USART2, USART_FLAG_ORE);
   }
 //******************************************************************	
     if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)  
       { 				  
  	    	Res =USART_ReceiveData(USART2);	//读取接收到的数据
	     	if((USART2_RX_STA&0x8000)==0)//接收未完成
		  	{
		    	if(USART2_RX_STA&0x4000)//接收到了0x0d
			   	{
			    	if(Res!=0x0a)USART2_RX_STA=0;//接收错误,重新开始
			    	else USART2_RX_STA|=0x8000;	//接收完成了 
				  }
		    	else //还没收到0X0D
			  	{	
			    	if(Res==0x0d)USART2_RX_STA|=0x4000;
			    	else
				  	{
				    	USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res ;
				    	USART2_RX_STA++;
				  	if(USART2_RX_STA>(USART2_REC_LEN-1))USART2_RX_STA=0;//接收数据错误,重新开始接收	  
					  }		 
				  }
		  	} 
       }   
				
	//******************防止串口死机******************************************
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {   
        USART_ClearFlag(USART2, USART_FLAG_RXNE);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        USART_ReceiveData(USART2);
    }
 //******************************************************************					
} 



 
void ALARM_DISPIAY(void) //SN号 报警显示 3.5寸屏幕
{
	static u8 err_flag_1=0xFF;//错误标志位 记录值
	static u16 Surplus_days_1=201;//错误标志位 记录值
	static u8 times1=0;
 if(err_flag==0)
 {
 if(info_1.Surplus_days!=Surplus_days_1)
 {
	 if(info_1.Surplus_days<200)
	 {
     LOCK_day[43]=info_1.Surplus_days/100+'0';
	   LOCK_day[44]=info_1.Surplus_days%100/10+'0';
     LOCK_day[45]=info_1.Surplus_days%10+'0';
		 UART2_PutStr(LOCK_day);
	 }else if(info_1.Surplus_days==1000)
	 {
		 times1=0;
		 UART2_PutStr(PAGE); //刷屏
	 }
	 Surplus_days_1=info_1.Surplus_days;
	 USART2_RX_STA=0;    //接收状态标记	清0 
 }
 }
 if(((err_flag!=0)&&(err_flag!=err_flag_1)&&(times1!=3))||(times1==0))
 {
	 times1=1;
	 SN_WRITE[42]=info_1.serialnum1; //字母
	 SN_WRITE[43]=info_1.serialnum2/10+'0';
	 SN_WRITE[44]=info_1.serialnum2%10+'0';
	 SN_WRITE[45]=info_1.serialnum3/10+'0';
	 SN_WRITE[46]=info_1.serialnum3%10+'0';
	 SN_WRITE[47]=info_1.serialnum4/10+'0';
	 SN_WRITE[48]=info_1.serialnum4%10+'0';
	 SN_WRITE[49]=info_1.serialnum5/100+'0';
	 SN_WRITE[50]=info_1.serialnum5%100/10+'0';
	 SN_WRITE[51]=info_1.serialnum5%100%10+'0';	 
   UART2_PutStr(SN_WRITE);
    delay_ms(2);
	 UART2_PutStr(SN_WRITE);
	 SN_POWER[41]=info_1.serialnum2/10+'0';
	 if(SN_POWER[41]=='0') SN_POWER[41]=' '; //高位是0输出空
	 SN_POWER[42]=info_1.serialnum2%10+'0';
	 UART2_PutStr(SN_POWER);
	 err_flag_1=err_flag;
	 USART2_RX_STA=0;    //接收状态标记	清0 
	 
	 	 if(err_flag==0)
	 {
		 if(info_1.lockflag==1) UART2_PutStr(lock); //锁定
	 }	 
	 else if(err_flag!=0XFF)
	 {
		 if(err_flag<=0X02) //PD
		 {
			 UART2_PutStr(PdCode1_2);
		 }
		 else if(err_flag==0X03) //QBH
		 {
			 UART2_PutStr(QbhCode3);
		 }
		 else if(err_flag<=0X07) //温度
		 {
			 UART2_PutStr(TCode4_7);
		 }
		 else if(err_flag<=13) //电流
		 {
			 UART2_PutStr(CCode8_13);
		 }
		 else if(err_flag==14) //水压
		 {
			 UART2_PutStr(FwCode14);
		 }
		 else if(err_flag==15) //急停
		 {
			 UART2_PutStr(StopCode15);
		 }
		 else if(err_flag==16) //参数改变
		 {
			 UART2_PutStr(numberCode16);
		 }
		 else if(err_flag==17) //PD锁机
		 {
			 UART2_PutStr(PD_continuity17);
			 times1=3;
		 }
		 
	 }
  }

	
}





