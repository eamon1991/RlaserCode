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
 
//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
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

//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码. 
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


//得到当前的时间
//返回值:0,成功;其他:错误代码.
u32 RTC_Get()  //得到当前时间
{
	static	u16	daycnt	=	0;
	u32	timecount	=	0;
	u32	temp	=	0;
	u16	temp1	=	0;
	u32 temp_val;
	
	temp_val = timecount	=	RTC_GetCounter();
	
	temp	=	timecount/86400; //得到天数(秒钟数对应的)
	
	if(daycnt	!= temp) //超过一天了
	{
		daycnt	=	temp;
		
		temp1	=	1970; //从1970年开始
		while(temp	>= 365) //闰年的秒钟数
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
		calendar.month	=	temp1+1;	 //得到月份	
		calendar.day		=	temp+1;	   //得到日期 
	}
	temp=timecount%86400;     		//得到秒钟数   	   
	calendar.hour=temp/3600;     	//小时
	calendar.min=(temp%3600)/60; 	//分钟	
	calendar.sec=(temp%3600)%60; 	//秒钟
	calendar.week		=	RTC_Get_Week(calendar.year, calendar.month, calendar.day);
	return temp_val;
}
//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
//月份数据表	
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
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号	
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
//RTC时钟中断
//每秒触发一次  
//extern u16 tcnt; 
 void RTC_IRQHandler(void)
 {	 
	 #if 0	
	 if(RTC_GetITStatus(RTC_IT_SEC)	!=	RESET)	RTC_Get();	
	 #endif
	 
	 if(RTC_GetITStatus(RTC_IT_ALR)	!=	RESET)	 //RTC定时 报警中断
	 {
			RTC_ClearITPendingBit(RTC_IT_ALR);
			RTC_WaitForLastTask();	
			
			lock_action();
		 
			RTC_ITConfig(RTC_IT_ALR	,DISABLE);
	 }
	 
	 RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_ALR |RTC_IT_OW);
	 RTC_WaitForLastTask();
 }
 
 
void lock_action(void) //PD连续报警 锁机 
{
	Write_Flash_Word(LockFlagAddr,1); //自锁标志位与上位机协同
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

 
//#define PD_continuity 0X11 //PD；连续报警
extern u8 err_flag;  //错误标�
 void power_on_lock_action(void) //时间到期后 锁定  //历史报警码读取
{
	static u8 i,im=0;
	static const u32 LockTime_Tab[6] = {(long)(1*30*24*60*60),(long)(2*30*24*60*60),(long)(3*30*24*60*60),(long)(4*30*24*60*60),(long)(5*30*24*60*60),(long)(6*30*24*60*60)};
	//static const u32 time_day1=(long)(1*24*60*60);
	static u32 time_now_count,lockttimeall=0;
	static u16 lock_type,lock_year,lock_month,lock_day,lock_hour,lock_min,lock_sec;
	time_now_count = RTC_Get();  //时间  实时计数
	RTC_WaitForSynchro();  //等带同步
	
	lock_type = Read_Flash_Word(LockTypeAddr);  //解密时间点
	lock_year = Read_Flash_Word(LockYearAddr);
	lock_month = Read_Flash_Word(LockMonthAddr);
	lock_day = Read_Flash_Word(LockDayAddr);
	lock_hour = Read_Flash_Word(LockHourAddr);
	lock_min = Read_Flash_Word(LockMinAddr);
	lock_sec =0;
	if((lock_type!=6)&&(lock_type!=7))
	{
		if(lock_type<6){ 	lockttimeall = get_secod_count(lock_year,lock_month,lock_day,lock_hour,lock_min,lock_sec) + LockTime_Tab[lock_type];}  //把激活日期转换为秒
		else 
     {	lockttimeall = get_secod_count(lock_year,lock_month,lock_day,lock_hour,lock_min,lock_sec) + time_day(lock_type-7);  //把激活日期转换为秒
		 }
		if(time_now_count >= lockttimeall)	lock_action(); //到期后锁定
		else{info_1.Surplus_days =(lockttimeall-time_now_count)/86400;} //剩余天数
	}
	else info_1.Surplus_days=1000;
	
		for(i=0;i<4;i++) //连续4次PD报警
	 {
		if(Read_Err_History((u8)i)==0X02) im++;  //向后addr 读取储存报警
		if(im==4)
		{
		  Send_Err(PD_continuity); ////发送PD连续报警提示
	  if(info_1.serialnum1!='T')
		{
			lock_action(); //最近5次PD2报警 锁死激光器
		  err_flag=0X11;  //错误标�
		}
			
		}
	 }


}
 
