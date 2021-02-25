#include "sys.h"


void timer_init()		
{
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1|RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM4|RCC_APB1Periph_TIM5,ENABLE);	
	
	TIM_TimeBaseStructure.TIM_Period	=	999;	 //T5 0.1S
	TIM_TimeBaseStructure.TIM_Prescaler	=	7199;
	TIM_TimeBaseStructure.TIM_ClockDivision	=	0;
	TIM_TimeBaseStructure.TIM_CounterMode		=	TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure); 
	TIM_Cmd(TIM5,ENABLE);
	TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
	NVIC_InitStructure.NVIC_IRQChannel	=	TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	=	0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	=	3;
	NVIC_InitStructure.NVIC_IRQChannelCmd	=	ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period	=	9;		 //T2 1ms
	TIM_TimeBaseStructure.TIM_Prescaler	=	7199;
	TIM_TimeBaseStructure.TIM_ClockDivision	=	0;
	TIM_TimeBaseStructure.TIM_CounterMode		=	TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure); 
	TIM_Cmd(TIM2,ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel	=	TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	=	2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	=	2;
	NVIC_InitStructure.NVIC_IRQChannelCmd	=	ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	#if 0
	TIM_TimeBaseStructure.TIM_Period	=	9999;	 //T4定时1s
	TIM_TimeBaseStructure.TIM_Prescaler	=	7199;
	TIM_TimeBaseStructure.TIM_ClockDivision	=	0;
	TIM_TimeBaseStructure.TIM_CounterMode		=	TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure); 
	TIM_Cmd(TIM4,ENABLE);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel	=	TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	=	2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	=	3;
	NVIC_InitStructure.NVIC_IRQChannelCmd	=	ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	#endif
	
	TIM_TimeBaseStructure.TIM_Period	=	4999;	 //T3定时0.5s
	TIM_TimeBaseStructure.TIM_Prescaler	=	7199;
	TIM_TimeBaseStructure.TIM_ClockDivision	=	0;
	TIM_TimeBaseStructure.TIM_CounterMode		=	TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure); 
	TIM_Cmd(TIM3,ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
	NVIC_InitStructure.NVIC_IRQChannel	=	TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	=	2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	=	3;
	NVIC_InitStructure.NVIC_IRQChannelCmd	=	ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = 7199;
	TIM_TimeBaseStructure.TIM_Prescaler =0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_Pulse = 0; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; 
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable); 
	TIM_ARRPreloadConfig(TIM1, ENABLE);
	TIM_Cmd(TIM1, ENABLE); 
	Set_PWM(10,0);
}

void Set_PWM(u32 frequence, u8 duty) //设定PWM 频率 占空比
{
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable); 
	TIM1->CNT = 0;
	TIM1->ARR = 72000/frequence-1;
	TIM1->CCR4 = 72000/frequence*duty/100;
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable); 
}

void Stop_PWM()  //停止PWM
{
	TIM1->ARR = 7199;
	TIM1->CCR4 = 0;
}



//static errSaveInfo ErrSaveInfo;
//static void ErrSave(u16 cod)
//{
//	u8 addr;
//	RTC_Get();
//	ErrSaveInfo.flag = 0x01;
//	ErrSaveInfo.year = calendar.year;
//	ErrSaveInfo.month = calendar.month;
//	ErrSaveInfo.day = calendar.day;
//	ErrSaveInfo.hour = calendar.hour;
//	ErrSaveInfo.min = calendar.min;
//	ErrSaveInfo.sec = calendar.sec;
//	ErrSaveInfo.errCode = cod;
//	addr = Read_Flash_Word(ErrGuideAddr);
//	switch(addr)
//	{
//		case 0x01:STMFLASH_Write(ErrInfo1Addr1,&ErrSaveInfo.flag,8);Write_Flash_Word(ErrGuideAddr,0x02); break;
//		case 0x02:STMFLASH_Write(ErrInfo2Addr1,&ErrSaveInfo.flag,8);Write_Flash_Word(ErrGuideAddr,0x03);break;
//		case 0x03:STMFLASH_Write(ErrInfo3Addr1,&ErrSaveInfo.flag,8);Write_Flash_Word(ErrGuideAddr,0x01); break;
//		default:break;
//	}
//}

u8 err_flag = 0;  //错误标志

static u8 Alarm_ON(u8 err_code)  //报警保护措施  err_code 报警代码
{
	Ext_Module_Disable();
	info_1.power=0;
	SW_On;
	Rty_Off;//断开继电器
	if(WorkMode==InMode)	Set_PWM(10,0);	else Module_Off;
	Set_LD_Volt(info_1.power);
	if(info_1.red_enable&&RED_ENABLE_IN)Red_guid_On;
	Alarm_LED_On;
	Run_LED_Off;
	err_flag = err_code;
	return 1;
}


u8 PD1_Err_Flag = 0; //错误标志
u8 PD2_Err_Flag = 0; //错误标志

extern u8 falling_flag; //下降沿标志
 u8 PD_Check(void)  //PD检测
{
	u8 isAlarm = 0;
	static u16 PD1_Code = 0;
	static u16 PD2_Code = 0;
	if(enable_alarm(PD2_Screen)&&enable_alarm(PD1_Screen)) //PD1 PD2都开启
	{
		if((falling_flag==0)&&(info_1.power>5)&&(info_1.current1>0.6))
		{
			PD2_Code	<<=	1;
			if((info_1.pd2_volt >= 3.25)&&(Pd1_in == 1))		  PD2_Code |= 0x01;	else	PD2_Code &= 0xFFFE;
			if(((PD2_Code&0xFFFF) == 0xFFFF))	{isAlarm=Alarm_ON(Pd2Code);}
		}
	}
	else
	{
	if(enable_alarm(PD2_Screen))
	{
		if((falling_flag==0)&&(info_1.power>5)&&(info_1.current1>0.6))
		{
			PD2_Code	<<=	1;
			if(info_1.pd2_volt >= 3.25)		  PD2_Code |= 0x01;	else	PD2_Code &= 0xFFFE;
			if(((PD2_Code&0xFFFF) == 0xFFFF))	{isAlarm=Alarm_ON(Pd2Code);}
		}
	}
	if(enable_alarm(PD1_Screen))
	{
		if((falling_flag==0)&&(info_1.power>5)&&(info_1.current1>0.6))
		{
		  if(Pd1_in == 1)PD1_Code |= 0x01;	else	PD1_Code &= 0xFFFE; 
	  	if(((PD1_Code&0xFFFF) == 0xFFFF))  {isAlarm = Alarm_ON(Pd1Code);}
		}
	}
  }
	
	return isAlarm;
}

static u8 QBH_Check()  //QBH检测  1.3s-2.2s左右 实际接近2.2s
{
	static  u8 times; 
	static  u8 QBH_50time=0;//50*1=50MS   50*1.7=85MS  
	u8 isAlarm = 0;
	if(times>24) //50MS*25=1.3S左右  85MS*25=2.2S左右
	{
		times=0;
	//if(Change_external_1.QBH_enable)
	if(enable_alarm(QBH_Screen))
	{
		if(QBH_in == 0)
		{
			delay_us(1);
			if(QBH_in == 0)		
			{
				QBH_50time++;
				if(QBH_50time>50)
				{
					QBH_50time=0;
				 isAlarm = Alarm_ON(QbhCode);
				}
			}
		}
		else
		{
			QBH_50time=0;
		}
	}
  }
	else times++;
	return isAlarm;
}

static u8 Flow_Check() //水压检测 6.5s-11s 左右 实际接近11s
{
	static  u16 times; 
	static u16 WatterFLAG =0;  //16次16*1=16ms 16*1.7=27ms
	u8 isAlarm = 0;
  if(times>400) //27ms*400=11s或6.5s
	{
	times=0;
	//if(Change_external_1.Flow_enable)
	if(enable_alarm(Flow_Screen))
	{
		WatterFLAG <<=	1;
		if(Flow_in == 1)
		{
			delay_us(1);
			if(Flow_in == 1)		
			{
				WatterFLAG |= 0x0001;
			}
			else	WatterFLAG &= 0xFFFE;
			
			if(((WatterFLAG&0xFFFF) == 0xFFFF))
				isAlarm = Alarm_ON(FwCode);
		}
	}
	}
	else times++;
	return isAlarm;
}

static u8 Temp_Check()  //温度检测
{
	u8 isAlarm = 0;
	u8 TempValue = info_1.Para.Tover_set;
	
	static u8 T1_Code =	0;
	static u8 T2_Code = 0;
	static u8 T3_Code = 0;
	static u8 T4_Code = 0;
	
	if(enable_alarm(Temp1_Screen))
	{
		T1_Code	<<=	1;
		if(info_1.temp1 >= TempValue)	T1_Code |= 0x01;	else	T1_Code &= 0xFE;
		if(((T1_Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(T1Code);}
	}
	if(enable_alarm(Temp2_Screen))
	{
		T2_Code	<<=	1;
		if(info_1.temp2 >= TempValue)	T2_Code |= 0x01;	else	T2_Code &= 0xFE;
		if(((T2_Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(T2Code);}
	}
	if(enable_alarm(Temp3_Screen))
	{
		T3_Code	<<=	1;
		if(info_1.temp3 >= TempValue)	T3_Code |= 0x01;	else	T3_Code &= 0xFE;
		if(((T3_Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(T3Code);}
	}
	if(enable_alarm(Temp4_Screen))
	{
		T4_Code	<<=	1;
		if(info_1.temp4 >= TempValue)	T4_Code |= 0x01;	else	T4_Code &= 0xFE;
		if(((T4_Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(T4Code);}
	}
	return isAlarm;
}

extern u8 drive_num; //驱动板个数

static u8 Current_Check() //电流检测
{
	u8 isAlarm = 0;
	float CoverValue = (float)(info_1.Para.Iover_set/10.0)/1.1;
	static u8 Cover1Code = 0;
	static u8 Cover2Code = 0;
	static u8 Cover3Code = 0;
	static u8 Cover4Code = 0;
	static u8 Cover5Code = 0;
	static u8 Cover6Code = 0;
	
	if(enable_alarm(Current1_Screen)&&(drive_num>=1))
	{
		Cover1Code	<<=	1;
		if(info_1.current1 >= CoverValue)	Cover1Code |= 0x01;	else	Cover1Code &= 0xFE;
		if(((Cover1Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(C1Code);}
	}
	
	if(enable_alarm(Current2_Screen)&&(drive_num>=2))
	{
		Cover2Code	<<=	1;
		if(info_1.current2 >= CoverValue)	Cover2Code |= 0x01;	else	Cover2Code &= 0xFE;
		if(((Cover2Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(C2Code);}
	}
	
	if(enable_alarm(Current3_Screen)&&(drive_num>=3))
	{
		Cover3Code	<<=	1;
		if(info_1.current3 >= CoverValue)	Cover3Code |= 0x01;	else	Cover3Code &= 0xFE;
		if(((Cover3Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(C3Code);}
	}
	
	if(enable_alarm(Current4_Screen)&&(drive_num>=4))
	{
		Cover4Code	<<=	1;
		if(info_1.current4 >= CoverValue)	Cover4Code |= 0x01;	else	Cover4Code &= 0xFE;
		if(((Cover4Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(C4Code);}
	}
	
	if(enable_alarm(Current5_Screen)&&(drive_num>=5))
	{
		Cover5Code	<<=	1;
		if(info_1.current5 >= CoverValue)	Cover5Code |= 0x01;	else	Cover5Code &= 0xFE;
		if(((Cover5Code&0xFF) == 0xFF))	{isAlarm=Alarm_ON(C5Code);}
	}
	
	if(enable_alarm(Current6_Screen)&&(drive_num>=6))
	{
		Cover6Code	<<=	1;
		if(info_1.current6 >= CoverValue)	Cover6Code |= 0x01;	else	Cover6Code &= 0xFE;
		if((Cover6Code&0xFF) == 0xFF)	{isAlarm=Alarm_ON(C6Code);}
	}
	
	return isAlarm;
}

static u8 Stop_Check()   //急停检测
{
	
	u8 isalarm = 0;
	if(Stop_in == 1)
	{
		delay_us(1);
		if(Stop_in == 1)
		{	
			//Time2_Disable();
			//Usart_Disable();
//			Ext_Module_Disable();
//			PD1_Disable();
//			PD2_Disable();
//			if(WorkMode == InMode)	Set_PWM(0,0);
//			else	Module_Off;
//			Rty_Off;
//			Alarm_LED_On;
//			Run_LED_Off;
//			Power_LED_On;
//			delay_ms(100);
//			TIM_Cmd(TIM4,DISABLE);
//			Buzzer_On;
			isalarm = Alarm_ON(StopCode);
		}
	}
	return isalarm;
}

static void ExtMode_Update()  //外控模式功率跟新 
{	
	if(WorkMode == ExtMode)
	{
		if(info_1.test_or_rule==1) //出货版==1) 
		{
			  if(info_1.power_gather >=100)	info_1.power = 100;
		    else info_1.power = info_1.power_gather; 
			  if(info_1.power>5)
				{
			   if(info_1.power<10) info_1.power_change=10;
				 else info_1.power_change=info_1.power;
				}
				else	info_1.power_change=0;
		    Set_LD_Volt(info_1.power_change);
		}
		else
			{
		    if(info_1.power_gather >=100)	info_1.power = 100;
		    else info_1.power = info_1.power_gather; 
		    Set_LD_Volt(info_1.power);
			}
	}
}

static void GetInfo()   //获取电流和温度 、外控功率
{
	Get_adc();
	Get_Current();
	ExtMode_Update();
}

static u8 Check_Alarm()  //报警检测
{
	u8 isAlarm_PD = 0,isAlarm_QBH = 0,isAlarm_Temp = 0,isAlarm_Current = 0,isAlarm_Flow = 0,isAlarm_Stop=0;

	isAlarm_Stop = Stop_Check();  //急停检测
	isAlarm_PD = PD_Check();    //PD检测
	isAlarm_QBH=QBH_Check();     //QBH检测
	isAlarm_Flow=Flow_Check();   //水压检测
	isAlarm_Temp=Temp_Check();
	isAlarm_Current=Current_Check();  //电流检测
	return (isAlarm_PD|isAlarm_QBH|isAlarm_Temp|isAlarm_Current|isAlarm_Flow|isAlarm_Stop);
}

 extern u8 Err_addr; //历史报警 读取次数  
//定时器2 中断    实际执行时间 大约1ms (不包含外控中断)
//实际6个电流模块 760ms 发送一次数据   中断1.7ms一次
//2000W  2ms刷新率
void TIM2_IRQHandler(void)  
{	
	if( TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		static u8 flag_0 = 0;
		static u8 flag = 0;
		static u8 isAlarm = 0;
		static u8 time_count =0;
		if(info_1.lockflag == 0) //解锁状态
		{
			GetInfo();  //获取电流和温度 、外控功率
			if(isAlarm ==0)	isAlarm = Check_Alarm();  //所有报警检测
		}
		if(time_count==100) //10*3.3ms， 33ms检测一次
			{
				if((isAlarm!=0)&&(flag==0)) 
		     {/*ErrSave(err_flag);*/
					 time_count=0;
		       Buzzer_On;TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
		       Send_Err(err_flag);flag = 1;
		     }				 
		 }
		if(time_count++==150)	//0.15s
       {
				   time_count = 0;
				if(Err_addr<21) // 150ms循环一次
				{
					Send_PC_Err_History(Err_addr); //连续发生MCU报警历史
		         Err_addr++;
				}
				else
				{
				if(flag_0==0) Send_Status(); flag_0++;if(flag_0 >= 3)flag_0= 0;
				}
			} 
			else	{	com_app();	}  //处理上位机 接收到的数据 //10ms一次
			
			if(Display) ALARM_DISPIAY(); //SN号 报警显示 3.5寸屏幕
			
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

#if 0
void TIM4_IRQHandler(void)	// 开关时间检测 /1S
{	
	static u16 Use_time_L,time_and=0; //分钟 ,时间++， time_and=120*0.5s=1min，记录一次
	if( TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{		
		 //开关时间检测
			if(info_1.power>=10) //NA>5%
			{
				time_and++;
				if(time_and>60) //1min
				{
					time_and=0;
					Use_time_L=Read_Flash_Word(Use_time_min);//读取到的使用时间 min
					if(Use_time_L<43200)//小于一个月
					{
						Use_time_L++;
						Write_Flash_Word(Use_time_min, Use_time_L);
					}
					else
					{
						Write_Flash_Word(Use_time_min, 0);//分钟清0
						Write_Flash_Word(Use_time_month,(Read_Flash_Word(Use_time_month)+1));// 读取到的使用月				
					}
				}
			}
	
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
}
#endif



void TIM5_IRQHandler(void)	 //报警信号 蜂鸣器 0.1s/
{	
	static u8 times=0; //延时写入报警记录
	static u8 Arm_times=0; 
	static u8 Lock_times=0; 
	if( TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		if(info_1.lockflag) //自锁报警
		{
			Lock_times++;
			if(Lock_times>=2)
			{
				Lock_times=0;
			PCout(7) = !PCout(7);
			PCout(12) = !PCout(12);
			}
		}
		else //普通报警
		{
			Arm_times++;
			if(Arm_times>=5) //0.5
			{Arm_times=0;
			PCout(7) = !PCout(7);
			}
			if(times<40)
			{		
				times++;
				if(times==25)	Write_Err_eeprom(err_flag); //报警码 MCU记录 延时25*0.1=2.5秒
			}
		}
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	}
}



void TIM3_IRQHandler(void)	
{	
	if( TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		info_1.power = 0;Module_Off; Set_LD_Volt(0);Run_LED_Off;if(info_1.red_enable&&RED_ENABLE_IN)Red_guid_On;
		WorkMode = InMode;Set_PA11_PWM();
		if(Module3_in == 1)
		{
			Set_PA11_IO();
			WorkMode = ExtMode;
			if(info_1.red_enable&&RED_ENABLE_IN) Red_guid_Off;
			Run_LED_Off;
			Module_On;
		}
		TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}

