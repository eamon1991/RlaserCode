#include "sys.h"

        //EXTI_Trigger_Rising        ---��ʾ��ѹ�Ӹ��½��������趨��ֵʱ�����жϣ� 
        //EXTI_Trigger_Falling       ---��ʾ��ѹ�ӵ������������趨��ֵʱ�����жϣ� 
        //EXTI_Trigger_Rising_Falling---��ʾ��ѹ�������½�Խ���趨��ֵʱ�������жϡ�


u32 num = 0;
 
void PWR_PVD_Init(void) 
{   
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
     
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//ʹ��PWRʱ��
 
    NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;           //ʹ��PVD���ڵ��ⲿ�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       //�����ȼ�0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          //ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure);
     
    EXTI_StructInit(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line = EXTI_Line16;             //PVD���ӵ��ж���16��
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     //ʹ���ж�ģʽ
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //��ѹ���ڷ�ֵʱ�����ж�
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;               //ʹ���ж���
    EXTI_Init(&EXTI_InitStructure);                         //��ʼ
     
    PWR_PVDLevelConfig(PWR_PVDLevel_2V9);//�趨��ط�ֵ
    PWR_PVDCmd(ENABLE);//ʹ��PVD     
}
 

static void Alarm_ON1(void)  //���籣��
{
	Ext_Module_Disable();
	info_1.power=0;
	SW_On;
	if(WorkMode==InMode)	Set_PWM(10,0);	else Module_Off;
	Set_LD_Volt(info_1.power);
	if(info_1.red_enable&&RED_ENABLE_IN)Red_guid_On;
	Run_LED_Off;
}

void PVD_IRQHandler(void)
{ 
    EXTI_ClearITPendingBit(EXTI_Line16);//���ж�
     
 //   num = BKP_ReadBackupRegister(BKP_DR10); //��ָ�����ݼĴ�����ȡ����
    num++;
     __disable_irq();//�ر����жϣ�
  
    //�û���ӽ���������봦
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);//ʹ��PWR��BKP����ʱ��
    PWR_BackupAccessCmd(ENABLE);//ʹ�ܺ󱸼Ĵ�������
    BKP_WriteBackupRegister(BKP_DR10, (u8)num);//�������� 
	  Alarm_ON1();  //���籣��
}

