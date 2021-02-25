#ifndef __SYS_H_
#define __SYS_H_

#include "stm32f10x.h"
#include "stdio.h"
#include "stdlib.h"




//**********�޸���******************************
#define Code_Edition	 100 //����汾S3.01
#define Display	       0  //����ʾ��=1       �� ������ʾ��=0
#define Resistance	   1   //����0.01=1��ʾ����������0.02=0
//	 info_1.test_or_rule  //�������%5-10%=10 =1   ���ʰ�ʵ�����(����) =0
//***************************************

#define time_day(day)    (u32)((day)*24*60*60)
#define OneMonth		(long)(1*30*24*60*60)
#define TwoMonth		(long)(2*30*24*60*60)
#define ThreeMonth	(long)(3*30*24*60*60)
#define FourMonth		(long)(4*30*24*60*60)
#define FiveMonth		(long)(5*30*24*60*60)
#define SixMonth 		(long)(6*30*24*60*60)

#define FLASH_SAVE_ADDR1	0X08018000  //�� 49������
#define ParaSetAddr 	(FLASH_SAVE_ADDR1+0)	//para info
#define ParaRedAddr		(FLASH_SAVE_ADDR1+2)
#define ParaPD1Addr		(FLASH_SAVE_ADDR1+4)
#define ParaPD2Addr 	(FLASH_SAVE_ADDR1+6)
#define ParaToverAddr	(FLASH_SAVE_ADDR1+8)
#define ParaIoverAddr	(FLASH_SAVE_ADDR1+10)
#define ParaImaxAddr	(FLASH_SAVE_ADDR1+12) //������
#define ParaScrAddr 	(FLASH_SAVE_ADDR1+14)


#define ErrGuideAddr	(FLASH_SAVE_ADDR1+16)	//err_guide
#define ErrInfo1Addr1	(FLASH_SAVE_ADDR1+18)	//err1 info
#define ErrInfo1Addr2	(FLASH_SAVE_ADDR1+20)
#define ErrInfo1Addr3	(FLASH_SAVE_ADDR1+22)
#define ErrInfo1Addr4	(FLASH_SAVE_ADDR1+24)
#define ErrInfo1Addr5	(FLASH_SAVE_ADDR1+26)
#define ErrInfo1Addr6	(FLASH_SAVE_ADDR1+28)
#define ErrInfo1Addr7	(FLASH_SAVE_ADDR1+30)
#define ErrInfo1Addr8	(FLASH_SAVE_ADDR1+32)
#define ErrInfo2Addr1	(FLASH_SAVE_ADDR1+34)	//err2 info
#define ErrInfo2Addr2	(FLASH_SAVE_ADDR1+36)
#define ErrInfo2Addr3	(FLASH_SAVE_ADDR1+38)
#define ErrInfo2Addr4	(FLASH_SAVE_ADDR1+40)
#define ErrInfo2Addr5	(FLASH_SAVE_ADDR1+42)
#define ErrInfo2Addr6	(FLASH_SAVE_ADDR1+44)
#define ErrInfo2Addr7	(FLASH_SAVE_ADDR1+46)
#define ErrInfo2Addr8	(FLASH_SAVE_ADDR1+48)
#define ErrInfo3Addr1	(FLASH_SAVE_ADDR1+50)	//err3 info
#define ErrInfo3Addr2	(FLASH_SAVE_ADDR1+52)
#define ErrInfo3Addr3	(FLASH_SAVE_ADDR1+54)
#define ErrInfo3Addr4	(FLASH_SAVE_ADDR1+56)
#define ErrInfo3Addr5	(FLASH_SAVE_ADDR1+58)
#define ErrInfo3Addr6	(FLASH_SAVE_ADDR1+60)
#define ErrInfo3Addr7	(FLASH_SAVE_ADDR1+62)
#define ErrInfo3Addr8	(FLASH_SAVE_ADDR1+64)

#define SeriaNumAddr1	(FLASH_SAVE_ADDR1+66)  //ϵ�к�
#define SeriaNumAddr2	(FLASH_SAVE_ADDR1+68)
#define SeriaNumAddr3	(FLASH_SAVE_ADDR1+70)
#define SeriaNumAddr4	(FLASH_SAVE_ADDR1+72)
#define SeriaNumAddr5	(FLASH_SAVE_ADDR1+74)
#define Locked_file   (FLASH_SAVE_ADDR1+76)  // �����ļ� ����


//temp |= (1 << 13)  &=~(1<<13) ˮѹ
//|=1<<2; &=~(1<<2);qbh

#define FLASH_SAVE_ADDR3	0X08018800  //�� 50������

#define FLASH_SAVE_ADDR4	0X08019000  //�� 51������
#define ADDR4_check 	(FLASH_SAVE_ADDR4+0)	// ������ʼ У���־ 0X11
#define ADDR4_QBH 	(FLASH_SAVE_ADDR4+4)	//
#define ADDR4_Flow 	(FLASH_SAVE_ADDR4+6)	//ˮѹ
#define LockFlagAddr	(FLASH_SAVE_ADDR4+8)	//������־
#define LockYearAddr	(FLASH_SAVE_ADDR4+10)
#define LockMonthAddr	(FLASH_SAVE_ADDR4+12)
#define LockDayAddr		(FLASH_SAVE_ADDR4+14)
#define LockHourAddr	(FLASH_SAVE_ADDR4+16)
#define LockMinAddr		(FLASH_SAVE_ADDR4+18)
#define LockSecAddr		(FLASH_SAVE_ADDR4+20)
#define FlashTestAddr	(FLASH_SAVE_ADDR4+22)	//flash test
#define LockTypeAddr	(FLASH_SAVE_ADDR4+24)	//�������� 1�� 2��
#define LockNumAddr   (FLASH_SAVE_ADDR4+26) //��������

#define RedStatusAddr	(FLASH_SAVE_ADDR4+28)  //���
#define usekey_times	(FLASH_SAVE_ADDR4+30) //��¼����

typedef struct
{
	u16 QBH_enable;//Change_external.QBH_enable
	u16 Flow_enable;
}Change_external;   //�ͻ��ⲿ�ɱ����
extern Change_external Change_external_1; //�ͻ��ⲿ�ɱ����

#define FLASH_SAVE_ADDR5	0X08019800  //�� 52������




 //0x0800FFFF��ֹ��64K��R8
//0x0801FFFF��ֹ��128K��RB 
//0X800(2048)һ������ ÿ������2K
#define FLASH_SAVE_ADDR2   0X08020000 	//�� 49+4������   ����FLASH �����ַ
#define Err_History_sign    0X08020000   //�ڼ���������¼��ַ
#define Err_History(N)			((u32)(FLASH_SAVE_ADDR2+2+12*N))  //MCU��ʷ��¼�����ַ   N��¼20��

//****����һ����������ֹ�͵�ѹ���������հ�*****************
#define Use_time_min        0X08021000  // �� 49+6������   ��λ�֣���ַ20������ʹ��ʱ�� ��43200 һ����
#define Use_time_month        0X08021002  //��λ��  ��ַ20������ʹ��ʱ��

//*******************************




void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);	
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead); 
u16 Read_Flash_Word(u32 ReadAddr);
void Write_Flash_Word(u32 WriteAddr,u16 WriteData);
void load_para(void);



enum workMode{InMode = 0,ExtMode};
extern enum workMode WorkMode;

#define IWDG_Feed()	IWDG_ReloadCounter()  //ι��

#define	IntPowerCmd	0x0101   // ���ڽ�������
#define SetRedCmd		0x0102
#define SetPd1Cmd		0x0103
#define SetPd2Cmd		0x0104  
#define SetToverCmd	0x0105
#define SetIoverCmd	0x0106
#define SetImaxCmd	0x0107
#define SetScrenCmd	0x0108
//#define UnclockCmd	0x0109
#define UnclockCmd	0x0F09
#define SetTimeCmd	0x010A
#define ReadInfoCmd	0x010B
#define ReadErrCmd	0x010C
#define SetMaxIxCmd	0x010D
#define WriteSpCmd	0x010E



#define DriveModule1 	0x80  //����ģ��INA220��ַ
#define DriveModule2 	0x82
#define DriveModule3 	0x84
#define DriveModule4 	0x86
#define DriveModule5 	0x88
#define DriveModule6	0x8A
#define DriveModule7	0x8C
#define DriveModule8 	0x8E
#define DriveModule9 	0x90
#define DriveModule10 0x92
#define DriveModule11 0x94
#define DriveModule12 0x96
#define DriveModule13 0x98
#define DriveModule14 0x9A
#define DriveModule15 0x9C
#define DriveModule16 0x9E


typedef struct
{
	u16 flag;
	u16 year;
	u16 month;
	u16 day;
	u16 hour;
	u16 min;
	u16 sec;
	u16 errCode;
}errSaveInfo;   //������Ϣ�����ʽ


typedef	struct
{
	u16 year;
	u16	month;
	u16	day;
	u16	hour;	
	u16 min;	
	u16 sec;			
	u16 week;		
}calendar_obj;  //ʱ���ʽ
extern calendar_obj	calendar;

typedef	struct
{
	u16 red_set;
	u16 pd1_set;
	u16 pd2_set;
	u16 Tover_set;
	u16 Iover_set;
	u16 Imax_set;
	u16	alarm_screen;
}para;   //���ò���

typedef struct
{
	u8 red_enable;
	
	u8 power_gather;
	u8 power; //���빦��
	u8 power_change; //�Ķ����ʣ�������11%  ��ʾ11% ʵ��power_display=15%
	u8 temp1;
	u8 temp2;
	u8 temp3;
	u8 temp4;
	float current1;
	float current2;
	float current3;
	float current4;
	float current5;
	float current6;
	float current7;
	float current8;
	float current9;
	float current10;
	float current11;
	float current12;
	float current13;
	float current14;
	float current15;
	float current16;
	float pd2_volt;
	u8 ld_volt;
	u8 pd_volt;
	para Para;
	
	u8 	lockflag;
	u8 	locknum;
	u8  locktype; //����� ������־
	u16 lockyear;
	u8 	lockmonth;
	u8 	lockday;
	u8 	lockhour;
	u8 	lockmin;
	u8 serialnum1;
	u8 serialnum2;
	u8 serialnum3;
	u8 serialnum4;
	u16 serialnum5;
	u16 Code_Editions; //����汾
	u8  key_time; //��¼���� ���255�� ���1 ���¿�ʼ
	u8 test_or_rule;//���� or ��������
	u16 Surplus_days; //����ʣ������
}Info;  //�豸״̬
extern Info info_1;
extern u8 random;

typedef struct
{
  u8  flag;
  u16 DestAddr;
  u16 SrcAddr;
  u8  Type;
  u8  Length;
  u8  buf[255];
  u8  verify;
}frmFmt; //�������ݸ�ʽ��֡��ʼ��Ŀ�ĵ�ַ��ԭ��ַ��֡���͡����ݳ��ȡ�������У����


enum cmdStatus{stateIdle = 0,stateStart, stateDest, stateSrc,
                stateType,stateLength, stateData, stateVerify};
							
#define RED_Channel	0   //DACоƬTLC5620���ͨ��
#define	LD_Channel	1
#define PD2_Channel	2
#define PD1_Channel	3


#define Pd2_in			PBin(0) 
#define Pd1_in			PBin(1)
#define Module3_in	PBin(3)
#define Module4_in	PBin(4)
#define Enable_in		PBin(5)
#define QBH_in			PBin(12)
#define Stop_in			PCin(8)
#define Start_in		PCin(9)
#define Flow_in			PCin(9)
#define RED_ENABLE_IN PAin(2)
//#define RED_ENABLE_IN 1

#define Buzzer_On			PCout(7)=1
#define Buzzer_Off		PCout(7)=0
#define Alarm_LED_On	PCout(12)=0
#define Alarm_LED_Off	PCout(12)=1
#define Run_LED_On		PCout(10)=1
#define Run_LED_Off		PCout(10)=0
#define Power_LED_On	PCout(11)=1
#define Power_LED_Off	PCout(11)=0
#define Rty_On				PDout(2)=0
#define Rty_Off				PDout(2)=1
#define Red_guid_On		PBout(13)=0  //���
#define Red_guid_Off	PBout(13)=1  //���
#define SW_On					PAout(12)=1
#define SW_Off				PAout(12)=0
#define Module_On			PAout(11)=1  //����
#define Module_Off		PAout(11)=0

#define Ext_Module_Disable()	{EXTI->IMR	&=  ~EXTI_Line3;EXTI->IMR	&=  ~EXTI_Line4;}  //�ر��ⲿ�ж� 3��4
#define Ext_Module_Enable()		{EXTI->IMR	|=  EXTI_Line3;EXTI->IMR	|=  EXTI_Line4;}  //ʹ���ⲿ�ж� 3 4
#define PD1_Disable()					{EXTI->IMR	&=  ~EXTI_Line0;}
#define PD1_Enable()					{EXTI->IMR	|=  EXTI_Line0;}
#define PD2_Disable()					{EXTI->IMR	&=  ~EXTI_Line1;}
#define PD2_Enable()					{EXTI->IMR	|=  EXTI_Line1;}
#define	Usart_Enable()				{USART1->CR1 |= ((uint16_t)0x2000) ;}
#define Usart_Disable()				{USART1->CR1 &= ((uint16_t)0xDFFF);}
#define Time2_Enable()				{TIM2->CR1 |= TIM_CR1_CEN;}
#define Time2_Disable()				{TIM2->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));}	

#define Set_PA11_IO()		{GPIOA->CRH &= 0xFFFF0FFF; GPIOA->CRH |= 0x00003000;}  //�������
#define Set_PA11_PWM()	{GPIOA->CRH &= 0xFFFF0FFF; GPIOA->CRH |= 0x0000B000;}   //��������

//�������δ��� Bit ����
#define enable_alarm(part)	((info_1.Para.alarm_screen&(1<<part)))?1:0
#define PD1_Screen			0  //������Ļͨ��
#define PD2_Screen			1
#define QBH_Screen			2   //QBH  0X04
#define Temp1_Screen		3
#define Temp2_Screen		4
#define Temp3_Screen		5
#define Temp4_Screen		6
#define Current1_Screen	7
#define Current2_Screen	8
#define Current3_Screen	9
#define Current4_Screen	10
#define Current5_Screen	11
#define Current6_Screen 12   
#define Flow_Screen			13  //ˮѹ 0X2000

#define Pd1Code	0x01  //PD1����
#define Pd2Code	0x02  //PD2����
#define QbhCode	0x03  //QBH����
#define T1Code	0x04  //�¶�1����
#define T2Code	0x05  //�¶�2����
#define T3Code	0x06  //�¶�3����
#define T4Code	0x07  //�¶�4����
#define C1Code	0x08  //����1����
#define C2Code	0x09  //����2����
#define C3Code	0x0A  //����3����
#define C4Code	0x0B  //����4����
#define C5Code	0x0C  //����5����
#define C6Code	0x0D  //����6����
#define FwCode  0x0E  //ˮѹ����
#define StopCode	0x0F //��ͣ����
#define numberCode	0x10 //�����ı�
#define PD_continuity 0X11 //PD����������

#define VOID_flash 0XFF //�� ����

void delay_init(void);
void delay_us(u32 n);
void delay_ms(u32 n);
void USART1_SendByte(u8 dat);
u8 get_crc(u8 *dat,u8 len);
void DMA_usart1_Send(void);
void usart1_init(void);
void adc_init(void);
void Get_adc(void);
u8 RTC_Init(void);
u8 RTC_Set(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec);
u32 get_secod_count(u16 year, u8 mon, u8 day, u8 hour, u8 min, u8 sec);
u32 RTC_Get(void);
u8 RTC_Get_Week(u16 year, u8 month, u8 day);
void tlc5620_init(void);
void TLC5620_Set_DA(u8 Addr,float volt);
void Get_Current(void);
void exti_init(void);
void timer_init(void);
void Set_PWM(u32 frequence, u8 duty);
void Stop_PWM(void);
void ina220_init(void);
u16	INA220_Read_Register( u8 slaveDevice, u8 reg_pointer);
void INA220_Init(void);        		  
u16 INA220_ReadOneWord(u8 device,u8 ReadAddr);
void INA220_WriteOneWord(u8 device,u8 WriteAddr,u16 DataToWrite);
float INA_ReadCurrent(u8 drive_select);
//float INA_ReadVoltage(void);
void Set_LD_Volt(u8 power);
void Send_Para(void);	
void Send_Screen(void);
void Send_WorkMode_Err(void);
void Send_Err(u8 err_code);
void Send_Status(void);
void Send_ErrSave(void);
void com_app(void);
u8 M24512_Check(void);
void eeprom_init(void);
 void lock_action(void);
 
 //�¼��� 3.5����ʾ������������
void USART2_Config(void);  //����2 3.5����ʾ
void ALARM_DISPIAY(void); //SN�� ������ʾ 3.5����Ļ
void Write_Err_eeprom(u8 err_code); //������ MCU��¼
void Send_PC_Err_History(u8 addr); //���ʹ洢 ��ʷ��������
 int crc16(u8 *puchMsg,u8 usDataLen);	 //*16CRCУ����*
 void Average_Current(void); //0.5S��50�ε���ƽ��
  void File_Lock_up(void);  //�ļ����� ��������
 void File_Lock_down(void);  //�ļ����� У��
  void power_on_lock_action(void); //ʱ�䵽�ں� ����  //��ʷ�������ȡ
	u8 Read_Err_History(u8 addr); //���addr ��ȡ���汨��;
void PWR_PVD_Init(void) ; //���籣��
 u8 PD_Check(void);  //PD���
 void adc_PDCheck(void); //PD�������
 void clean_Err_eeprom(void); //������ ���
#endif
