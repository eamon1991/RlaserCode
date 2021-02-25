#include "sys.h"
#include "stdlib.h"

#define STM32_FLASH_SIZE 256 	 		//ËùÑ¡STM32µÄFLASHÈİÁ¿´óĞ¡(µ¥Î»ÎªK)
#define STM32_FLASH_WREN 1              //Ê¹ÄÜFLASHĞ´Èë(0£¬²»ÊÇÄÜ;1£¬Ê¹ÄÜ)
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASHµÄÆğÊ¼µØÖ·
//////////////////////////////////////////////////////////////////////////////////
 //0x0800FFFF½ØÖ¹£¨64K£©R8
//0x0801FFFF½ØÖ¹£¨128K£©RB 

//¶ÁÈ¡Ö¸¶¨µØÖ·µÄ°ë×Ö(16Î»Êı¾İ)
//faddr:¶ÁµØÖ·(´ËµØÖ·±ØĞëÎª2µÄ±¶Êı!!)
//·µ»ØÖµ:¶ÔÓ¦Êı¾İ.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//Èç¹ûÊ¹ÄÜÁËĞ´   
//²»¼ì²éµÄĞ´Èë
//WriteAddr:ÆğÊ¼µØÖ·
//pBuffer:Êı¾İÖ¸Õë
//NumToWrite:°ë×Ö(16Î»)Êı   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//µØÖ·Ôö¼Ó2.
	}  
} 
//´ÓÖ¸¶¨µØÖ·¿ªÊ¼Ğ´ÈëÖ¸¶¨³¤¶ÈµÄÊı¾İ
//WriteAddr:ÆğÊ¼µØÖ·(´ËµØÖ·±ØĞëÎª2µÄ±¶Êı!!)
//pBuffer:Êı¾İÖ¸Õë
//NumToWrite:°ë×Ö(16Î»)Êı(¾ÍÊÇÒªĞ´ÈëµÄ16Î»Êı¾İµÄ¸öÊı.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //×Ö½Ú
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//×î¶àÊÇ2K×Ö½Ú

void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //ÉÈÇøµØÖ·
	u16 secoff;	   //ÉÈÇøÄÚÆ«ÒÆµØÖ·(16Î»×Ö¼ÆËã)
	u16 secremain; //ÉÈÇøÄÚÊ£ÓàµØÖ·(16Î»×Ö¼ÆËã)	   
 	u16 i;    
	u32 offaddr;   //È¥µô0X08000000ºóµÄµØÖ·
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//·Ç·¨µØÖ·
	FLASH_Unlock();						//½âËø
	offaddr=WriteAddr-STM32_FLASH_BASE;		//Êµ¼ÊÆ«ÒÆµØÖ·.
	secpos=offaddr/STM_SECTOR_SIZE;			//ÉÈÇøµØÖ·  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//ÔÚÉÈÇøÄÚµÄÆ«ÒÆ(2¸ö×Ö½ÚÎª»ù±¾µ¥Î».)
	secremain=STM_SECTOR_SIZE/2-secoff;		//ÉÈÇøÊ£Óà¿Õ¼ä´óĞ¡   
	if(NumToWrite<=secremain)secremain=NumToWrite;//²»´óÓÚ¸ÃÉÈÇø·¶Î§
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//¶Á³öÕû¸öÉÈÇøµÄÄÚÈİ
		for(i=0;i<secremain;i++)//Ğ£ÑéÊı¾İ
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//ĞèÒª²Á³ı  	  
		}
		if(i<secremain)//ĞèÒª²Á³ı
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//²Á³ıÕâ¸öÉÈÇø
			for(i=0;i<secremain;i++)//¸´ÖÆ
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//Ğ´ÈëÕû¸öÉÈÇø  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//Ğ´ÒÑ¾­²Á³ıÁËµÄ,Ö±½ÓĞ´ÈëÉÈÇøÊ£ÓàÇø¼ä. 				   
		if(NumToWrite==secremain)break;//Ğ´Èë½áÊøÁË
		else//Ğ´ÈëÎ´½áÊø
		{
			secpos++;				//ÉÈÇøµØÖ·Ôö1
			secoff=0;				//Æ«ÒÆÎ»ÖÃÎª0 	 
		   	pBuffer+=secremain;  	//Ö¸ÕëÆ«ÒÆ
			WriteAddr+=secremain;	//Ğ´µØÖ·Æ«ÒÆ	   
		   	NumToWrite-=secremain;	//×Ö½Ú(16Î»)Êıµİ¼õ
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//ÏÂÒ»¸öÉÈÇø»¹ÊÇĞ´²»Íê
			else secremain=NumToWrite;//ÏÂÒ»¸öÉÈÇø¿ÉÒÔĞ´ÍêÁË
		}	 
	}	
	FLASH_Lock();//ÉÏËø
}
#endif

//´ÓÖ¸¶¨µØÖ·¿ªÊ¼¶Á³öÖ¸¶¨³¤¶ÈµÄÊı¾İ
//ReadAddr:ÆğÊ¼µØÖ·
//pBuffer:Êı¾İÖ¸Õë
//NumToWrite:°ë×Ö(16Î»)Êı
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//¶ÁÈ¡2¸ö×Ö½Ú.
		ReadAddr+=2;//Æ«ÒÆ2¸ö×Ö½Ú.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:ÆğÊ¼µØÖ·
//WriteData:ÒªĞ´ÈëµÄÊı¾İ
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




extern u8 err_flag;  //´íÎó±êÖ
void load_para()  //Ğ´ÈëÉèÖÃ
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
		Write_Flash_Word(ParaSetAddr,0x01);//²ÎÊıĞ´Èë±êÖ¾
		Write_Flash_Word(LockNumAddr,0);
		Write_Flash_Word(SeriaNumAddr1, 'A');
		Write_Flash_Word(SeriaNumAddr2, 0);
		Write_Flash_Word(SeriaNumAddr3, 0);
		Write_Flash_Word(SeriaNumAddr4, 0);
		Write_Flash_Word(SeriaNumAddr5, 0);
		Write_Flash_Word(usekey_times, 0);// //µÇÂ¼´ÎÊı
		Write_Flash_Word(ADDR4_check,51);// ÉÈÇø51
		Write_Flash_Word(ADDR4_QBH,1);
		Write_Flash_Word(ADDR4_Flow,1);
		STMFLASH_Write(Use_time_min,time,2);//¼¤¹âÆ÷Ê¹ÓÃÊ±¼ä³õÊ¼»¯=0X0000 0000
	}
	
	if(Read_Flash_Word(ADDR4_check)!=51) //ÉÈÇøÄÚÈİ¶ªÊ§
	{
		Write_Flash_Word(ADDR4_check,51);// ÉÈÇø51
		Write_Flash_Word(ADDR4_QBH,1);
		Write_Flash_Word(ADDR4_Flow,1);
		Write_Flash_Word(RedStatusAddr,1); //ºì¹â¿ª¹Ø	
		Write_Flash_Word(LockTypeAddr, 0);	//¼¤»îÀàĞÍ1¸öÔÂ
		RTC_Get();  //Ê±¼ä  ÊµÊ±¼ÆÊı
	  RTC_WaitForSynchro();  //µÈ´øÍ¬²½	
		Write_Flash_Word(LockNumAddr, 2);//´ÎÊı
	  Write_Flash_Word(LockYearAddr, calendar.year);   //ÊµÊ±Ê±ÖÓ
	  Write_Flash_Word(LockMonthAddr,calendar.month);
		Write_Flash_Word(LockDayAddr,  calendar.day);
		Write_Flash_Word(LockHourAddr, calendar.hour);
		Write_Flash_Word(LockMinAddr,  calendar.min);	
		Write_Flash_Word(LockFlagAddr,0);//½âËø×´Ì¬
	}
	
	info_1.red_enable = Read_Flash_Word(RedStatusAddr);      //ºì¹â¿ª¹ØÊ¹ÄÜ
	info_1.Para.red_set = Read_Flash_Word(ParaRedAddr);      //ºì¹âµçÁ÷
	info_1.Para.pd1_set = Read_Flash_Word(ParaPD1Addr);      //PD1 ²Î¿¼µçÑ¹
	info_1.Para.pd2_set = Read_Flash_Word(ParaPD2Addr);      //PD2 ²Î¿¼µçÑ¹
	info_1.Para.Tover_set = Read_Flash_Word(ParaToverAddr);  //ÎÂ¶ÈãĞÖµ
	info_1.Para.Iover_set = Read_Flash_Word(ParaIoverAddr);  //µçÁ÷ãĞÖµ
	info_1.Para.Imax_set = Read_Flash_Word(ParaImaxAddr);    //Êä³öµçÁ÷×î´óÖµ
	info_1.Para.alarm_screen = Read_Flash_Word(ParaScrAddr); //±¨¾¯Âë ÆÁ±Î±êÖ¾   &(~0X2004)//Ë®Ñ¹ QBHÈ¥³öÈ¥
	info_1.serialnum1 = (u8)Read_Flash_Word(SeriaNumAddr1); //SN
	info_1.serialnum2 = (u8)Read_Flash_Word(SeriaNumAddr2);
	info_1.serialnum3 = (u8)Read_Flash_Word(SeriaNumAddr3);
	info_1.serialnum4 = (u8)Read_Flash_Word(SeriaNumAddr4);
	info_1.serialnum5 = (u16)Read_Flash_Word(SeriaNumAddr5);
  File_Lock_down();  //ÎÄ¼ş²ÎÊı Ğ£Ñé

	info_1.locktype = Read_Flash_Word(LockTypeAddr);  //×ÔËø×´Ì¬	
	info_1.locknum = Read_Flash_Word(LockNumAddr);  //×ÔËø´ÎÊı
	info_1.lockflag = Read_Flash_Word(LockFlagAddr); //×ÔËø±êÖ¾
	info_1.lockyear = Read_Flash_Word(LockYearAddr);
	info_1.lockmonth = Read_Flash_Word(LockMonthAddr);
	info_1.lockday = Read_Flash_Word(LockDayAddr);
	info_1.lockhour = Read_Flash_Word(LockHourAddr);
	info_1.lockmin = Read_Flash_Word(LockMinAddr);
	
	info_1.key_time=(u8)Read_Flash_Word(usekey_times);// //µÇÂ¼´ÎÊı
	info_1.Code_Editions=(u16)Code_Edition; //´úÂë°æ±¾
	if(info_1.serialnum1=='T') info_1.test_or_rule=0;//²âÊÔ°æ±¾orÕı³£
	else info_1.test_or_rule=1;//²âÊÔ°æ±¾orÕı³£
	TLC5620_Set_DA(RED_Channel,(float)(info_1.Para.red_set*4/1000.0));
	TLC5620_Set_DA(PD1_Channel,(float)(info_1.Para.pd1_set/10.0));
	TLC5620_Set_DA(PD2_Channel,(float)(info_1.Para.pd2_set/10.0));
	TLC5620_Set_DA(LD_Channel,0);
	
	if((info_1.red_enable == 1)&&(RED_ENABLE_IN == 1)) Red_guid_On;	else Red_guid_Off;	//¼ÓÈëºì¹âÊ¹ÄÜ
	info_1.Surplus_days=201; //Ä¬ÈÏ201Ìì²»ÏÔÊ¾
	Change_external_1.QBH_enable=Read_Flash_Word(ADDR4_QBH);
	Change_external_1.Flow_enable=Read_Flash_Word(ADDR4_Flow);
}



/**********16Î»CRCĞ£Ñé*******/

//////////////¸ßÎ»±í///////////////////
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

//////////////µÍÎ»±í///////////////////
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

////////*16CRCĞ£Ñé²é±í·¢*////////////////
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

void Alarm_ON2(void)  //±¨¾¯±£»¤´ëÊ©  
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
void File_Lock_up(void)  //ÎÄ¼ş²ÎÊı ÉÏËøº¯Êı
{
  u16 CRC_16;  //ÎÄ¼şËø ¡¢Ğ£ÑéÎ» 
	u8 crc[30];
      
	crc[0]=(u8)(info_1.Para.red_set>>8);     //ºì¹âµçÁ÷
	crc[1]=(u8)info_1.Para.red_set;  
  crc[2]=(u8)(info_1.Para.pd1_set>>8);     //PD1 ²Î¿¼µçÑ¹  
	crc[3]=(u8)info_1.Para.pd1_set;  
	crc[4]=(u8)(info_1.Para.pd2_set>>8);      //PD2 ²Î¿¼µçÑ¹
	crc[5]=(u8)info_1.Para.pd2_set;  
  crc[6]=(u8)(info_1.Para.Tover_set>>8);    //ÎÂ¶ÈãĞÖµ
	crc[7]=(u8)info_1.Para.Tover_set;    
	crc[8]=(u8)(info_1.Para.Iover_set>>8);    //µçÁ÷ãĞÖµ
	crc[9]=(u8)info_1.Para.Iover_set;   
	crc[10]=(u8)(info_1.Para.Imax_set>>8);      //Êä³öµçÁ÷×î´óÖµ
	crc[11]=(u8)info_1.Para.Imax_set;    
	crc[12]=(u8)((info_1.Para.alarm_screen&(~0X2004))>>8); //±¨¾¯Âë ÆÁ±Î±êÖ¾   &(~0X2004)//Ë®Ñ¹ QBHÈ¥³öÈ¥
	crc[13]=(u8)(info_1.Para.alarm_screen&(~0X2004)); 
	crc[14]=info_1.serialnum1; //SN
	crc[15]=info_1.serialnum2; //SN
	crc[16]=info_1.serialnum3; //SN
  crc[17]=info_1.serialnum4; //SN
	crc[18]=(u8)(info_1.serialnum5>>8); //SN
	crc[19]=(u8)info_1.serialnum5; //SN
	CRC_16=crc16(crc,20);
	Write_Flash_Word(Locked_file,CRC_16);  //²ÎÊıĞ£Ñé´¢´æ
}



void File_Lock_down(void)  //ÎÄ¼ş²ÎÊı Ğ£Ñé
{
  u16 File_EN, CRC_16;  //ÎÄ¼şËø ¡¢Ğ£ÑéÎ» 
	u8 crc[30];
	File_EN= Read_Flash_Word(Locked_file); // ²ÎÊıÎÄ¼ş ÉÏËø      
	crc[0]=(u8)(info_1.Para.red_set>>8);     //ºì¹âµçÁ÷
	crc[1]=(u8)info_1.Para.red_set;  
  crc[2]=(u8)(info_1.Para.pd1_set>>8);     //PD1 ²Î¿¼µçÑ¹  
	crc[3]=(u8)info_1.Para.pd1_set;  
	crc[4]=(u8)(info_1.Para.pd2_set>>8);      //PD2 ²Î¿¼µçÑ¹
	crc[5]=(u8)info_1.Para.pd2_set;  
  crc[6]=(u8)(info_1.Para.Tover_set>>8);    //ÎÂ¶ÈãĞÖµ
	crc[7]=(u8)info_1.Para.Tover_set;    
	crc[8]=(u8)(info_1.Para.Iover_set>>8);    //µçÁ÷ãĞÖµ
	crc[9]=(u8)info_1.Para.Iover_set;   
	crc[10]=(u8)(info_1.Para.Imax_set>>8);      //Êä³öµçÁ÷×î´óÖµ
	crc[11]=(u8)info_1.Para.Imax_set;    
	crc[12]=(u8)((info_1.Para.alarm_screen&(~0X2004))>>8); //±¨¾¯Âë ÆÁ±Î±êÖ¾   &(~0X2004)//Ë®Ñ¹ QBHÈ¥³öÈ¥
	crc[13]=(u8)(info_1.Para.alarm_screen&(~0X2004)); 
	crc[14]=info_1.serialnum1; //SN
	crc[15]=info_1.serialnum2; //SN
	crc[16]=info_1.serialnum3; //SN
  crc[17]=info_1.serialnum4; //SN
	crc[18]=(u8)(info_1.serialnum5>>8); //SN
	crc[19]=(u8)info_1.serialnum5; //SN
	CRC_16=crc16(crc,20);
	if(CRC_16!=File_EN)  //Êı¾İ±»¸Ä  Ö»±¨¾¯ĞÅºÅ
	{
		Send_Err(numberCode);
		Alarm_ON2();  //±¨¾¯±£»¤´ëÊ©  
		Buzzer_On;
		TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);//±¨¾¯Ö¸Ê¾µÆ
		err_flag=0x10;  //´íÎó±êÖ
	//	if(Display) ALARM_DISPIAY(); //SNºÅ ±¨¾¯ÏÔÊ¾ 3.5´çÆÁÄ»
	}

}





