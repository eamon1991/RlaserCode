#include "sys.h"

#define N	8		 //ADC 8ͨ��
#define M	150  //����150��
#define ADC1_DR_Address   ((u32)0x40012400+0x4c)    //ADC��ַ
#define GetVolt(ad_value)	((ad_value)*3.3/4096.0)  //ADC����ֵ ת����ѹ
#define GetPower(v)				(u8)((v)*104/2.2)      //���ݵ�ѹֵ���㹦��
#define GetRes(v)					((v>2.9)&&(v<=3.3))?Res_Tab[24]:((u16)((10100*(v)-330)/(3.3-(v))))  //��������ֵ ���ݵ�ѹת����ֵ 
	
static u16 AD_Value[M][N];  //ADC�洢��

const	u16 Res_Tab[71]	=	{32660, 31040, 29500, 28060, 26680, 25400, 24180, 23020, 21920, 20880, //0-9
													 19900, 18970, 18290, 17260, 16460, 15710, 15000, 14320, 13680, 13070, //10-19
													 12490, 11940, 11420, 10920, 10450, 10000, 9574,  9166,	 8778,  8480,	 //20-29
													 8058,  7724,  7404,  7098,  6808,	 6532,  6268,  6015,  5776,  5546,	 //30-39
													 5326,  5118,  4918,  4726,  4544,  4368,  4202,  4042,  3888,  3742,	 //40-49
													 3602,  3468,  3340,  3216,  3098,  2986,  2878,  2774,  2674,  2580,	 //50-59
													 2488,  2400,	 2316,  2234,  2158,  2082,  2012,  1942,  1876,  1813,	 //60-69
													 1751 /*70*/ };   //����-�¶� ���ձ�


void adc_init()
{
	ADC_InitTypeDef	ADC_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1|RCC_APB2Periph_ADC2,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);	
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);	  //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_0;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_4|GPIO_Pin_5;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	DMA_DeInit(DMA1_Channel1);///��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr=(u32)&AD_Value;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴�����ADC���͵��ڴ��ȡ
	DMA_InitStructure.DMA_BufferSize = N*M;    //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;    //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;   //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode =  DMA_Mode_Circular;  //ѭ������ģʽ
	DMA_InitStructure.DMA_Priority =  DMA_Priority_High;  //DMAͨ�� xӵ�и����ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1,&DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
	DMA_Cmd(DMA1_Channel1,ENABLE);
	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode	=	ADC_Mode_Independent;	     //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode	=	ENABLE;	          //���ŵ�ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode	=	ENABLE;  //ģ��ת������������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv	=	ADC_ExternalTrigConv_None; //ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign	=	ADC_DataAlign_Right; //ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel	=	8; 	//˳����й���ת����ADCͨ������Ŀ���˴���6���ŵ����ɿ���Ϊ1~16��
	//ADC�����ŵ�����
//ADC1,ADCͨ��x,�������˳��ֵΪy,����ʱ��Ϊ239.5����
	ADC_Init(ADC1,&ADC_InitStructure);
	ADC_RegularChannelConfig( ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig( ADC1, ADC_Channel_2, 2, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig( ADC1, ADC_Channel_3, 3, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig( ADC1, ADC_Channel_6, 4, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig( ADC1, ADC_Channel_7, 5, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig( ADC1, ADC_Channel_14,6, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig( ADC1, ADC_Channel_15,7, ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig( ADC1, ADC_Channel_8,8, ADC_SampleTime_55Cycles5);
	ADC_DMACmd(ADC1,ENABLE);
	ADC_Cmd(ADC1,ENABLE);	
	ADC_ResetCalibration(ADC1);	
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);	
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	
	
	//
	GPIO_InitStructure.GPIO_Pin	=	GPIO_Pin_2;	//ld_ctr,ld_cs
	GPIO_InitStructure.GPIO_Mode	=	GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed	=	GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}

#if 0
static void sort(u16 *dat) //�����㷨
{
	u8 i,j;
	u16 d_temp;
	for(i=0;i<M;i++)
	{
		for(j=i+1;j<M;j++)
		{
			if(dat[i] > dat[j])
			{
				d_temp	=	dat[i];
				dat[i]	=	dat[j];
				dat[j]	=	d_temp;
			}
		}
	}
}
#endif

static u8 temp_calcuate(float v_res)	 //���ݵ��� ��ȡ�¶� ���
{
	u16 res_value;	
	u8 i, l_num = 0 ,h_num = 70,m_num = 0;

	res_value = GetRes(v_res);
	
	for(i = 0; i<7; i++)
	{
			m_num	=	(u8)((l_num + h_num)/2);
			
			if(res_value > Res_Tab[m_num])
			{
				h_num	=	(l_num + h_num)/2;
				l_num	=	l_num;
			}
			else
			{
				l_num	=	(l_num+h_num)/2;
				h_num	=	h_num;
			}
	}
	return h_num;
}

void Get_adc() //��ȡͨ��ֵ ����ÿ��ͨ��ֵ ת���ɵ�ѹ �����豸״̬
{
	u32 i,sum =0;
	sum =0; for(i=0;i<20;i++)	sum+=AD_Value[i][0];	info_1.power_gather = GetPower(GetVolt((sum/20)));
	//sum =0; for(i=0;i<20;i++)	sum+=AD_Value[i][1];	info_1.ld_volt = GetVolt((sum/20));
	//sum =0; for(i=0;i<20;i++)	sum+=AD_Value[i][2];	info_1.pd_volt = GetVolt((sum/20));
	sum =0; for(i=0;i<20;i++)	sum+=AD_Value[i][3];	info_1.temp1 = temp_calcuate(GetVolt((sum/20)));
	sum =0; for(i=0;i<20;i++)	sum+=AD_Value[i][4];	info_1.temp2 = temp_calcuate(GetVolt((sum/20)));
	sum =0; for(i=0;i<20;i++)	sum+=AD_Value[i][5];	info_1.temp3 = temp_calcuate(GetVolt((sum/20)));
	sum =0; for(i=0;i<20;i++)	sum+=AD_Value[i][6];	info_1.temp4 = temp_calcuate(GetVolt((sum/20)));
	sum =0; for(i=0;i<M;i++)	sum+=AD_Value[i][7];	info_1.pd2_volt = GetVolt((sum/M));
	if(WorkMode == ExtMode)	info_1.power = info_1.power_gather;	
}






