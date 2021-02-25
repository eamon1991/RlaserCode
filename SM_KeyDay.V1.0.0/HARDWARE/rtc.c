#include "sys.h"


static const u16 Data_Flag = 0x55AA;	
static const u16 time_now[] = {2017,12,26,10,20,40};
static u8 const	table_week[12]	=	{0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};	
static u8 const	mon_table[12]		=	{31,28,31,30,31,30,31,31,30,31,30,31};
calendar_obj	calendar;

u8 RTC_Init()
{
	u8	temp	=	0;
	u32	timecount;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP,	ENABLE);	
	
	PWR_BackupAccessCmd(ENABLE);	
	
	if(BKP_ReadBackupRegister(BKP_DR1)	!=	Data_Flag)
	{				
		BKP_DeInit();	
		
		RCC_LSEConfig(RCC_LSE_ON);	
		
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY)	==	RESET)	
		{
			temp++;
			delay_ms(10);
		}
		if(temp>=250)	return 1;	
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	
		
		RCC_RTCCLKCmd(ENABLE);
		
		RTC_WaitForLastTask();
		
		RTC_WaitForSynchro();	
		
		RTC_ITConfig(RTC_IT_SEC,DISABLE);	
		
		RTC_ITConfig(RTC_IT_ALR,DISABLE);
		
		RTC_WaitForLastTask();
		
		RTC_EnterConfigMode();
		
		RTC_SetPrescaler(32767);
		
		RTC_WaitForLastTask();
		
		RTC_Set(time_now[0],time_now[1],time_now[2],time_now[3],time_now[4],time_now[5]); 
		
		RTC_ExitConfigMode();	
		
		timecount	=	RTC_GetCounter();
		
		RTC_SetAlarm(timecount+ThreeMonth);
		
		BKP_WriteBackupRegister(BKP_DR1, Data_Flag);
	}
//	else
//	{
//		RTC_WaitForSynchro();
//		RTC_ITConfig(RTC_IT_SEC ,DISABLE);
//		RTC_ITConfig(RTC_IT_ALR	,DISABLE);
//		timecount	=	RTC_GetCounter();
//		RTC_SetAlarm(timecount+Lock_Time);
//		RTC_WaitForLastTask();
//	}
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	RTC_Get();
	
	return 0;
}
 
//ÅĞ¶ÏÊÇ·ñÊÇÈòÄêº¯Êı
//ÔÂ·İ   1  2  3  4  5  6  7  8  9  10 11 12
//ÈòÄê   31 29 31 30 31 30 31 31 30 31 30 31
//·ÇÈòÄê 31 28 31 30 31 30 31 31 30 31 30 31
//ÊäÈë:Äê·İ
//Êä³ö:¸ÃÄê·İÊÇ²»ÊÇÈòÄê.1,ÊÇ.0,²»ÊÇ
static u8	Is_Leap_Year(u16 year)	
{
	if(year%4	==	0)
	{
			if(year%100	==	0)
			{
				if(year%400	==	0)	return	1;
				else	return 0;
			}else	return 1;
	}else	return 0;
}

//ÉèÖÃÊ±ÖÓ
//°ÑÊäÈëµÄÊ±ÖÓ×ª»»ÎªÃëÖÓ
//ÒÔ1970Äê1ÔÂ1ÈÕÎª»ù×¼
//1970~2099ÄêÎªºÏ·¨Äê·İ
//·µ»ØÖµ:0,³É¹¦;ÆäËû:´íÎó´úÂë. 
u8 RTC_Set(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec)	//set time
{
	u16 t;
	u32	seccount	=	0;
	
	if(year<1970 ||	year>2099)	return 1;
	
	for(t=1970; t<year; t++)
	{
		if(Is_Leap_Year(t))	seccount+=31622400;
		else	seccount+=31536000;
	}
	for(t=0;t<mon-1;t++)
	{
		seccount+=(u32)mon_table[t]*86400;
		if(Is_Leap_Year(year)&&t==1)	seccount+=86400;
	}
	seccount+=(u32)(day-1)*86400;
	seccount+=(u32)hour*3600;	
	seccount+=(u32)min*60;
	seccount+=(u32)sec;	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP,ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	RTC_SetCounter(seccount);
	RTC_WaitForLastTask();
	RTC_Get();
	return 0;
}


//µÃµ½µ±Ç°µÄÊ±¼ä
//·µ»ØÖµ:0,³É¹¦;ÆäËû:´íÎó´úÂë.
u32 RTC_Get()  //µÃµ½µ±Ç°Ê±¼ä
{
	static	u16	daycnt	=	0;
	u32	timecount	=	0;
	u32	temp	=	0;
	u16	temp1	=	0;
	u32 temp_val;
	
	temp_val = timecount	=	RTC_GetCounter();
	
	temp	=	timecount/86400; //µÃµ½ÌìÊı(ÃëÖÓÊı¶ÔÓ¦µÄ)
	
	if(daycnt	!= temp) //³¬¹ıÒ»ÌìÁË
	{
		daycnt	=	temp;
		
		temp1	=	1970; //´Ó1970Äê¿ªÊ¼
		while(temp	>= 365) //ÈòÄêµÄÃëÖÓÊı
		{
			if(Is_Leap_Year(temp1))
			{
				if(temp>=366)	temp-=366;
				else{temp1++;break;}
			}
			else	temp-=365;
			temp1++;
		}
		calendar.year	=	temp1;
		
		temp1	=	0;
		while(temp>=28)
		{
			if(Is_Leap_Year(calendar.year) && temp1==1)
			{
				if(temp>=29)	temp-=29;
				else	break;
			}
			else
			{
				if(temp>=mon_table[temp1])	temp	-= mon_table[temp1];
				else break;
			}
			temp1++;
		}
		calendar.month	=	temp1+1;	 //µÃµ½ÔÂ·İ	
		calendar.day		=	temp+1;	   //µÃµ½ÈÕÆÚ 
	}
	temp=timecount%86400;     		//µÃµ½ÃëÖÓÊı   	   
	calendar.hour=temp/3600;     	//Ğ¡Ê±
	calendar.min=(temp%3600)/60; 	//·ÖÖÓ	
	calendar.sec=(temp%3600)%60; 	//ÃëÖÓ
	calendar.week		=	RTC_Get_Week(calendar.year, calendar.month, calendar.day);
	return temp_val;
}
//ÉèÖÃÊ±ÖÓ
//°ÑÊäÈëµÄÊ±ÖÓ×ª»»ÎªÃëÖÓ
//ÒÔ1970Äê1ÔÂ1ÈÕÎª»ù×¼
//1970~2099ÄêÎªºÏ·¨Äê·İ
//·µ»ØÖµ:0,³É¹¦;ÆäËû:´íÎó´úÂë.
//ÔÂ·İÊı¾İ±í	
u32 get_secod_count(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec)
{
	u16 t;
	u32	seccount	=	0;
	
	if(year<1970 ||	year>2099)	return 1;
	
	for(t=1970; t<year; t++)	//year
	{
		if(Is_Leap_Year(t))	seccount+=31622400;
		else	seccount+=31536000;
	}
	for(t=0;t<mon-1;t++)	//month
	{
		seccount+=(u32)mon_table[t]*86400;
		if(Is_Leap_Year(year)&&t==1)	seccount+=86400;
	}
	seccount+=(u32)(day-1)*86400;	//day
	seccount+=(u32)hour*3600;	//hour
	seccount+=(u32)min*60;		//min
	seccount+=(u32)sec;			//second
	
	return seccount;
}
//»ñµÃÏÖÔÚÊÇĞÇÆÚ¼¸
//¹¦ÄÜÃèÊö:ÊäÈë¹«ÀúÈÕÆÚµÃµ½ĞÇÆÚ(Ö»ÔÊĞí1901-2099Äê)
//ÊäÈë²ÎÊı£º¹«ÀúÄêÔÂÈÕ 
//·µ»ØÖµ£ºĞÇÆÚºÅ	
u8	RTC_Get_Week(u16 year, u8 month, u8 day)
{
	u16	temp2;
	u8 	yearH,yearL;
	
	yearH	=	year/100;
	yearL	=	year%100;
	
	if(yearH	>19)	yearL+=100;
	temp2	=	yearL	+	yearL/4;
	temp2	=	temp2%7;
	temp2	=	temp2 + day	+table_week[month-1];
	if(yearL%4==0 && month<3)	temp2--;
	return (temp2%7);
}
//RTCÊ±ÖÓÖĞ¶Ï
//Ã¿Ãë´¥·¢Ò»´Î  
//extern u16 tcnt; 
 void RTC_IRQHandler(void)
 {	 
	 #if 0	
	 if(RTC_GetITStatus(RTC_IT_SEC)	!=	RESET)	RTC_Get();	
	 #endif
	 
	 if(RTC_GetITStatus(RTC_IT_ALR)	!=	RESET)	 //RTC¶¨Ê± ±¨¾¯ÖĞ¶Ï
	 {
			RTC_ClearITPendingBit(RTC_IT_ALR);
			RTC_WaitForLastTask();	
			
			lock_action();
		 
			RTC_ITConfig(RTC_IT_ALR	,DISABLE);
	 }
	 
	 RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_ALR |RTC_IT_OW);
	 RTC_WaitForLastTask();
 }
 
 
void lock_action(void) //PDÁ¬Ğø±¨¾¯ Ëø»ú 
{
	Write_Flash_Word(LockFlagAddr,1); //×ÔËø±êÖ¾Î»ÓëÉÏÎ»»úĞ­Í¬
	info_1.lockflag = Read_Flash_Word(LockFlagAddr);
	Ext_Module_Disable();
	Set_PWM(10,0);	Module_Off;
	info_1.power=0;
	Set_LD_Volt(info_1.power);
	//if(deviceInfo.MainStatus.Red_Shield)	Red_guid_On;	
	Red_guid_On;
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	Run_LED_Off;
}

 
//#define PD_continuity 0X11 //PD£»Á¬Ğø±¨¾¯
extern u8 err_flag;  //´íÎó±êÖ
 void power_on_lock_action(void) //Ê±¼äµ½ÆÚºó Ëø¶¨  //ÀúÊ·±¨¾¯Âë¶ÁÈ¡
{
	static u8 i,im=0;
	static const u32 LockTime_Tab[6] = {(long)(1*30*24*60*60),(long)(2*30*24*60*60),(long)(3*30*24*60*60),(long)(4*30*24*60*60),(long)(5*30*24*60*60),(long)(6*30*24*60*60)};
	//static const u32 time_day1=(long)(1*24*60*60);
	static u32 time_now_count,lockttimeall=0;
	static u16 lock_type,lock_year,lock_month,lock_day,lock_hour,lock_min,lock_sec;
	time_now_count = RTC_Get();  //Ê±¼ä  ÊµÊ±¼ÆÊı
	RTC_WaitForSynchro();  //µÈ´øÍ¬²½
	
	lock_type = Read_Flash_Word(LockTypeAddr);  //½âÃÜÊ±¼äµã
	lock_year = Read_Flash_Word(LockYearAddr);
	lock_month = Read_Flash_Word(LockMonthAddr);
	lock_day = Read_Flash_Word(LockDayAddr);
	lock_hour = Read_Flash_Word(LockHourAddr);
	lock_min = Read_Flash_Word(LockMinAddr);
	lock_sec =0;
	if((lock_type!=6)&&(lock_type!=7))
	{
		if(lock_type<6){ 	lockttimeall = get_secod_count(lock_year,lock_month,lock_day,lock_hour,lock_min,lock_sec) + LockTime_Tab[lock_type];}  //°Ñ¼¤»îÈÕÆÚ×ª»»ÎªÃë
		else 
     {	lockttimeall = get_secod_count(lock_year,lock_month,lock_day,lock_hour,lock_min,lock_sec) + time_day(lock_type-7);  //°Ñ¼¤»îÈÕÆÚ×ª»»ÎªÃë
		 }
		if(time_now_count >= lockttimeall)	lock_action(); //µ½ÆÚºóËø¶¨
		else{info_1.Surplus_days =(lockttimeall-time_now_count)/86400;} //Ê£ÓàÌìÊı
	}
	else info_1.Surplus_days=1000;
	
		for(i=0;i<4;i++) //Á¬Ğø4´ÎPD±¨¾¯
	 {
		if(Read_Err_History((u8)i)==0X02) im++;  //Ïòºóaddr ¶ÁÈ¡´¢´æ±¨¾¯
		if(im==4)
		{
		  Send_Err(PD_continuity); ////·¢ËÍPDÁ¬Ğø±¨¾¯ÌáÊ¾
	  if(info_1.serialnum1!='T')
		{
			lock_action(); //×î½ü5´ÎPD2±¨¾¯ ËøËÀ¼¤¹âÆ÷
		  err_flag=0X11;  //´íÎó±êÖ
		}
			
		}
	 }


}
 
