#include "sys.h"
#include "stdlib.h"

#define STM32_FLASH_SIZE 256 	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 1              //ʹ��FLASHд��(0��������;1��ʹ��)
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH����ʼ��ַ
//////////////////////////////////////////////////////////////////////////////////
 //0x0800FFFF��ֹ��64K��R8
//0x0801FFFF��ֹ��128K��RB 

//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�

void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	FLASH_Unlock();						//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain;	//д��ַƫ��	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	}	
	FLASH_Lock();//����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Write_Flash_Word(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);
}



u16 Read_Flash_Word(u32 ReadAddr)
{
	u16 dat[1];
	STMFLASH_Read(ReadAddr,dat,1);
	return dat[0];
}




extern u8 err_flag;  //������
void load_para()  //д������
{	

	if(Read_Flash_Word(ParaSetAddr) == 0xFFFF)	
	{	
		u16 time[2]={0,0};
		info_1.Para.red_set = 70;
		info_1.Para.pd1_set = 18;
		info_1.Para.pd2_set = 17;
		info_1.Para.Tover_set = 40;
		info_1.Para.Iover_set = 200;
		info_1.Para.Imax_set = 110;
		info_1.Para.alarm_screen = 0xFF;
		info_1.red_enable = 1;
		info_1.lockflag = 0;
		info_1.locknum = 0;
		info_1.locktype = 7;
		
		Write_Flash_Word(RedStatusAddr, info_1.red_enable);
		Write_Flash_Word(ParaRedAddr,info_1.Para.red_set);
		Write_Flash_Word(ParaPD1Addr,info_1.Para.pd1_set);
		Write_Flash_Word(ParaPD2Addr,info_1.Para.pd2_set);
		Write_Flash_Word(ParaToverAddr,info_1.Para.Tover_set);
		Write_Flash_Word(ParaIoverAddr,info_1.Para.Iover_set);
		Write_Flash_Word(ParaImaxAddr,info_1.Para.Imax_set);
		Write_Flash_Word(ParaScrAddr,info_1.Para.alarm_screen);
		Write_Flash_Word(LockFlagAddr,info_1.lockflag);
		Write_Flash_Word(LockNumAddr,info_1.locknum);
		Write_Flash_Word(LockTypeAddr,info_1.locktype);
		Write_Flash_Word(LockFlagAddr,0);
		Write_Flash_Word(LockYearAddr,2018);
		Write_Flash_Word(LockMonthAddr,8);
		Write_Flash_Word(LockDayAddr,21);
		Write_Flash_Word(LockHourAddr,14);
		Write_Flash_Word(LockMinAddr,0);
		Write_Flash_Word(ErrGuideAddr,0x01);
		Write_Flash_Word(ParaSetAddr,0x01);//����д���־
		Write_Flash_Word(LockNumAddr,0);
		Write_Flash_Word(SeriaNumAddr1, 'A');
		Write_Flash_Word(SeriaNumAddr2, 0);
		Write_Flash_Word(SeriaNumAddr3, 0);
		Write_Flash_Word(SeriaNumAddr4, 0);
		Write_Flash_Word(SeriaNumAddr5, 0);
		Write_Flash_Word(usekey_times, 0);// //��¼����
		Write_Flash_Word(ADDR4_check,51);// ����51
		Write_Flash_Word(ADDR4_QBH,1);
		Write_Flash_Word(ADDR4_Flow,1);
		STMFLASH_Write(Use_time_min,time,2);//������ʹ��ʱ���ʼ��=0X0000 0000
	}
	
	if(Read_Flash_Word(ADDR4_check)!=51) //�������ݶ�ʧ
	{
		Write_Flash_Word(ADDR4_check,51);// ����51
		Write_Flash_Word(ADDR4_QBH,1);
		Write_Flash_Word(ADDR4_Flow,1);
		Write_Flash_Word(RedStatusAddr,1); //��⿪��	
		Write_Flash_Word(LockTypeAddr, 0);	//��������1����
		RTC_Get();  //ʱ��  ʵʱ����
	  RTC_WaitForSynchro();  //�ȴ�ͬ��	
		Write_Flash_Word(LockNumAddr, 2);//����
	  Write_Flash_Word(LockYearAddr, calendar.year);   //ʵʱʱ��
	  Write_Flash_Word(LockMonthAddr,calendar.month);
		Write_Flash_Word(LockDayAddr,  calendar.day);
		Write_Flash_Word(LockHourAddr, calendar.hour);
		Write_Flash_Word(LockMinAddr,  calendar.min);	
		Write_Flash_Word(LockFlagAddr,0);//����״̬
	}
	
	info_1.red_enable = Read_Flash_Word(RedStatusAddr);      //��⿪��ʹ��
	info_1.Para.red_set = Read_Flash_Word(ParaRedAddr);      //������
	info_1.Para.pd1_set = Read_Flash_Word(ParaPD1Addr);      //PD1 �ο���ѹ
	info_1.Para.pd2_set = Read_Flash_Word(ParaPD2Addr);      //PD2 �ο���ѹ
	info_1.Para.Tover_set = Read_Flash_Word(ParaToverAddr);  //�¶���ֵ
	info_1.Para.Iover_set = Read_Flash_Word(ParaIoverAddr);  //������ֵ
	info_1.Para.Imax_set = Read_Flash_Word(ParaImaxAddr);    //����������ֵ
	info_1.Para.alarm_screen = Read_Flash_Word(ParaScrAddr); //������ ���α�־   &(~0X2004)//ˮѹ QBHȥ��ȥ
	info_1.serialnum1 = (u8)Read_Flash_Word(SeriaNumAddr1); //SN
	info_1.serialnum2 = (u8)Read_Flash_Word(SeriaNumAddr2);
	info_1.serialnum3 = (u8)Read_Flash_Word(SeriaNumAddr3);
	info_1.serialnum4 = (u8)Read_Flash_Word(SeriaNumAddr4);
	info_1.serialnum5 = (u16)Read_Flash_Word(SeriaNumAddr5);
  File_Lock_down();  //�ļ����� У��

	info_1.locktype = Read_Flash_Word(LockTypeAddr);  //����״̬	
	info_1.locknum = Read_Flash_Word(LockNumAddr);  //��������
	info_1.lockflag = Read_Flash_Word(LockFlagAddr); //������־
	info_1.lockyear = Read_Flash_Word(LockYearAddr);
	info_1.lockmonth = Read_Flash_Word(LockMonthAddr);
	info_1.lockday = Read_Flash_Word(LockDayAddr);
	info_1.lockhour = Read_Flash_Word(LockHourAddr);
	info_1.lockmin = Read_Flash_Word(LockMinAddr);
	
	info_1.key_time=(u8)Read_Flash_Word(usekey_times);// //��¼����
	info_1.Code_Editions=(u16)Code_Edition; //����汾
	if(info_1.serialnum1=='T') info_1.test_or_rule=0;//���԰汾or����
	else info_1.test_or_rule=1;//���԰汾or����
	TLC5620_Set_DA(RED_Channel,(float)(info_1.Para.red_set*4/1000.0));
	TLC5620_Set_DA(PD1_Channel,(float)(info_1.Para.pd1_set/10.0));
	TLC5620_Set_DA(PD2_Channel,(float)(info_1.Para.pd2_set/10.0));
	TLC5620_Set_DA(LD_Channel,0);
	
	if((info_1.red_enable == 1)&&(RED_ENABLE_IN == 1)) Red_guid_On;	else Red_guid_Off;	//������ʹ��
	info_1.Surplus_days=201; //Ĭ��201�첻��ʾ
	Change_external_1.QBH_enable=Read_Flash_Word(ADDR4_QBH);
	Change_external_1.Flow_enable=Read_Flash_Word(ADDR4_Flow);
}



/**********16λCRCУ��*******/

//////////////��λ��///////////////////
char const auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
} ;

//////////////��λ��///////////////////
char const auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
} ;

////////*16CRCУ����*////////////////
int crc16(u8 *puchMsg,u8 usDataLen) 	
{ 											
	char uchCRCHi = 0xFF ; 				
	char uchCRCLo = 0xFF ; 				
	int uIndex ; 							
	while (usDataLen--) 					
	{
		uIndex = uchCRCHi ^ *puchMsg++ ; 	
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
		uchCRCLo = auchCRCLo[uIndex] ;
	}
	return (((int)(uchCRCLo) << 8) | uchCRCHi) ;
}	

void Alarm_ON2(void)  //����������ʩ  
{
	Ext_Module_Disable();
	info_1.power=0;
	SW_On;
	if(WorkMode==InMode)	Set_PWM(10,0);	else Module_Off;
	Set_LD_Volt(info_1.power);
	if(info_1.red_enable&&RED_ENABLE_IN)Red_guid_On;
	Alarm_LED_On;
	Run_LED_Off;
}
void File_Lock_up(void)  //�ļ����� ��������
{
  u16 CRC_16;  //�ļ��� ��У��λ 
	u8 crc[30];
      
	crc[0]=(u8)(info_1.Para.red_set>>8);     //������
	crc[1]=(u8)info_1.Para.red_set;  
  crc[2]=(u8)(info_1.Para.pd1_set>>8);     //PD1 �ο���ѹ  
	crc[3]=(u8)info_1.Para.pd1_set;  
	crc[4]=(u8)(info_1.Para.pd2_set>>8);      //PD2 �ο���ѹ
	crc[5]=(u8)info_1.Para.pd2_set;  
  crc[6]=(u8)(info_1.Para.Tover_set>>8);    //�¶���ֵ
	crc[7]=(u8)info_1.Para.Tover_set;    
	crc[8]=(u8)(info_1.Para.Iover_set>>8);    //������ֵ
	crc[9]=(u8)info_1.Para.Iover_set;   
	crc[10]=(u8)(info_1.Para.Imax_set>>8);      //����������ֵ
	crc[11]=(u8)info_1.Para.Imax_set;    
	crc[12]=(u8)((info_1.Para.alarm_screen&(~0X2004))>>8); //������ ���α�־   &(~0X2004)//ˮѹ QBHȥ��ȥ
	crc[13]=(u8)(info_1.Para.alarm_screen&(~0X2004)); 
	crc[14]=info_1.serialnum1; //SN
	crc[15]=info_1.serialnum2; //SN
	crc[16]=info_1.serialnum3; //SN
  crc[17]=info_1.serialnum4; //SN
	crc[18]=(u8)(info_1.serialnum5>>8); //SN
	crc[19]=(u8)info_1.serialnum5; //SN
	CRC_16=crc16(crc,20);
	Write_Flash_Word(Locked_file,CRC_16);  //����У�鴢��
}



void File_Lock_down(void)  //�ļ����� У��
{
  u16 File_EN, CRC_16;  //�ļ��� ��У��λ 
	u8 crc[30];
	File_EN= Read_Flash_Word(Locked_file); // �����ļ� ����      
	crc[0]=(u8)(info_1.Para.red_set>>8);     //������
	crc[1]=(u8)info_1.Para.red_set;  
  crc[2]=(u8)(info_1.Para.pd1_set>>8);     //PD1 �ο���ѹ  
	crc[3]=(u8)info_1.Para.pd1_set;  
	crc[4]=(u8)(info_1.Para.pd2_set>>8);      //PD2 �ο���ѹ
	crc[5]=(u8)info_1.Para.pd2_set;  
  crc[6]=(u8)(info_1.Para.Tover_set>>8);    //�¶���ֵ
	crc[7]=(u8)info_1.Para.Tover_set;    
	crc[8]=(u8)(info_1.Para.Iover_set>>8);    //������ֵ
	crc[9]=(u8)info_1.Para.Iover_set;   
	crc[10]=(u8)(info_1.Para.Imax_set>>8);      //����������ֵ
	crc[11]=(u8)info_1.Para.Imax_set;    
	crc[12]=(u8)((info_1.Para.alarm_screen&(~0X2004))>>8); //������ ���α�־   &(~0X2004)//ˮѹ QBHȥ��ȥ
	crc[13]=(u8)(info_1.Para.alarm_screen&(~0X2004)); 
	crc[14]=info_1.serialnum1; //SN
	crc[15]=info_1.serialnum2; //SN
	crc[16]=info_1.serialnum3; //SN
  crc[17]=info_1.serialnum4; //SN
	crc[18]=(u8)(info_1.serialnum5>>8); //SN
	crc[19]=(u8)info_1.serialnum5; //SN
	CRC_16=crc16(crc,20);
	if(CRC_16!=File_EN)  //���ݱ���  ֻ�����ź�
	{
		Send_Err(numberCode);
		Alarm_ON2();  //����������ʩ  
		Buzzer_On;
		TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);//����ָʾ��
		err_flag=0x10;  //������
	//	if(Display) ALARM_DISPIAY(); //SN�� ������ʾ 3.5����Ļ
	}

}





