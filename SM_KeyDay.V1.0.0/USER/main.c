#include "sys.h"

enum workMode WorkMode = InMode; 	//����ģʽ
Info info_1;   //�豸״̬
u8 receive_flag = 0; //����һ֡���ݽ���״̬ 0û��� 1���
frmFmt frmFmt_1;   //�������ݽṹ��
extern u16 alarm_screen_part; //����������
extern u8 Tx_Buf[]; //���ڷ�������

static void gpio_init()
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);												
	
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_11|GPIO_Pin_12;	//ld_ctr,ld_cs
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	Module_Off;SW_Off;
	
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_13;	//red
	GPIO_Init(GPIOB,&GPIO_InitStructure);Red_guid_Off;
	
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_7|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//buzzer,led1,led2,led3
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	Alarm_LED_Off;
	
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_2;	//rty
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	Rty_Off;
	
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_12;	//pd2,pd1,module_1,moudle_2,nc+,qbh
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_IN_FLOATING; //��������
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_8|GPIO_Pin_9;	//stop,start
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_2;	//led enables
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	Alarm_LED_On;
	Run_LED_On;
	Power_LED_On;
	delay_ms(500);
	
	
	Buzzer_Off;
	
	delay_ms(500);
	delay_ms(500);
	Alarm_LED_Off;
	Run_LED_Off;
	Rty_On;
}

static void IWDG_Config(uint8_t prv ,uint16_t rlv)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(prv);
	IWDG_SetReload(rlv);
	IWDG_ReloadCounter();
	IWDG_Enable();	
}


u8 random;

int main()
{	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	delay_init();
	
	gpio_init();
	
	usart1_init();
		
	tlc5620_init();
	
	load_para(); 				//��FLASH �������� 
		
	while(RTC_Init()); 	//RTC ʵʱʱ��
	
	adc_init();	 				//ADC��ʼ��
	
  //if(Display) USART2_Config();  //����2 3.5����ʾ
	
	delay_ms(1000);
	
	INA220_Init(); 			// IIC�����ɼ�ģ��ת��
	
	timer_init();  			//��ʱ����ʼ��
	
	//if(Module3_in == 1) {WorkMode = ExtMode;Set_PA11_IO(); Module_On;Red_guid_Off;Run_LED_On;Get_adc();}else {Module_Off;Red_guid_On;Run_LED_Off;}
	
	exti_init();  			//�ⲿ�ж�
	
	power_on_lock_action(); //ʱ�䵽�ں� ����
   //PWR_PVD_Init() ; //���籣��
	 
	if(Read_Flash_Word(LockFlagAddr) == 1) TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	
	while(1) { delay_ms(10);}

	
}

