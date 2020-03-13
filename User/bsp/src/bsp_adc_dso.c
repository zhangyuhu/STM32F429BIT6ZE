/*
*********************************************************************************************************
*
*	ģ������ : ʾ����ģ��ADC�ײ������
*	�ļ����� : bsp_adc_dso.c
*	��    �� : V1.0
*	˵    �� : ʹ��STM32�ڲ�ADC��ͬ���ɼ���·���Ρ�ռ���˲���GPIO����ʾ����ģ����������Ϸ�ʽ��
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2015-10-06  armfly  ��ʽ����
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	STM32-V6�����壬ʾ����ģ��GPIO����

	---------------------------------------------------------
	D112-1 �汾��ʾ����ģ�� ���ɰ棩
		ʾ����ͨ��1
			PC0 = ADC123_IN10
			AC1 = PG10  ����AC-DC�л�, 1��ʾDC, 0��ʾAC
			G1  = PC6   ����˥����, 1��ʾ1:1�� 0��ʾ1:5

		ʾ����ͨ��2
			PF10 = ADC3_IN8
			AC2  = PB7  ����AC-DC�л�, 1��ʾDC, 0��ʾAC
			G2   = PC7  ����˥����, 1��ʾ1:1�� 0��ʾ1:5

	---------------------------------------------------------
	D112-2 �汾��ʾ����ģ��  ���°棩
		ʾ����ͨ��1
			PC3  = ADC123_IN13
			AC1  = PB7    ����AC-DC�л�, 1��ʾDC, 0��ʾAC
			G1A  = PC6    ����˥����
			G1B  = PC7    ����˥����

		ʾ����ͨ��2
			PC0  = ADC123_IN10
			AC2  = PA6   ����AC-DC�л�, 1��ʾDC, 0��ʾAC
			G2A  = PG10  ����˥����
			G2B  = PA5   ����˥����		
*/

#ifdef	D112_1
	#define GPIO_AC1	GPIOG
	#define PIN_AC1     GPIO_Pin_10
	#define RCC_AC1     RCC_AHB1Periph_GPIOG
	
	#define GPIO_AC2	GPIOB
	#define PIN_AC2     GPIO_Pin_7
	#define RCC_AC2     RCC_AHB1Periph_GPIOB
	
	#define GPIO_G1A	 GPIOC
	#define PIN_G1A      GPIO_Pin_6
	#define RCC_G1A      RCC_AHB1Periph_GPIOC
	#define GPIO_G1B	 GPIOC
	#define PIN_G1B      GPIO_Pin_6
	#define RCC_G1B      RCC_AHB1Periph_GPIOC	
	
	#define GPIO_G2A	 GPIOC
	#define PIN_G2A      GPIO_Pin_7
	#define RCC_G2A      RCC_AHB1Periph_GPIOC
	#define GPIO_G2B	 GPIOC
	#define PIN_G2B      GPIO_Pin_7
	#define RCC_G2B      RCC_AHB1Periph_GPIOC	
#else	
	#define GPIO_AC1	GPIOB
	#define PIN_AC1		GPIO_Pin_7
	#define RCC_AC1     RCC_AHB1Periph_GPIOB
	
	#define GPIO_AC2	GPIOA
	#define PIN_AC2		GPIO_Pin_6
	#define RCC_AC2     RCC_AHB1Periph_GPIOF
	
	#define GPIO_G1A	GPIOC
	#define PIN_G1A		GPIO_Pin_6
	#define RCC_G1A     RCC_AHB1Periph_GPIOC
	
	#define GPIO_G1B	GPIOC
	#define PIN_G1B		GPIO_Pin_7
	#define RCC_G1B     RCC_AHB1Periph_GPIOC
	
	#define GPIO_G2A	GPIOG
	#define PIN_G2A		GPIO_Pin_10
	#define RCC_G2A     RCC_AHB1Periph_GPIOG
	
	#define GPIO_G2B	GPIOA
	#define PIN_G2B		GPIO_Pin_5
	#define RCC_G2B     RCC_AHB1Periph_GPIOA
#endif	
	
#define AC1_0()		GPIO_AC1->BSRRH = PIN_AC1
#define AC1_1()		GPIO_AC1->BSRRL = PIN_AC1

#define AC2_0()		GPIO_AC2->BSRRH = PIN_AC2
#define AC2_1()		GPIO_AC2->BSRRL = PIN_AC2

#define G1A_0()		GPIO_G1A->BSRRH = PIN_G1A
#define G1A_1()		GPIO_G1A->BSRRL = PIN_G1A

#define G1B_0()		GPIO_G1B->BSRRH = PIN_G1B
#define G1B_1()		GPIO_G1B->BSRRL = PIN_G1B

#define G2A_0()		GPIO_G2A->BSRRH = PIN_G2A
#define G2A_1()		GPIO_G2A->BSRRL = PIN_G2A

#define G2B_0()		GPIO_G2B->BSRRH = PIN_G2B
#define G2B_1()		GPIO_G2B->BSRRL = PIN_G2B


/* ADCͨ������ */
#define GPIO_CH1	GPIOC
#define PIN_CH1     GPIO_Pin_3
#define RCC_CH1     RCC_AHB1Periph_GPIOC
#define CHAN_CH1	ADC_Channel_13

#define GPIO_CH2	GPIOC
#define PIN_CH2     GPIO_Pin_0
#define RCC_CH2     RCC_AHB1Periph_GPIOC
#define CHAN_CH2	ADC_Channel_10

/*
*********************************************************************************************************
*	�� �� ��: DSO_ConfigCtrlGPIO
*	����˵��: ���ÿ�����ͨ����Ϻ������GPIO
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DSO_ConfigCtrlGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AC1 | RCC_AC2 | RCC_G1A | RCC_G2A | RCC_G2A | RCC_G2B, ENABLE);
	
	/* ��������Ϊ�������ģʽ */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = PIN_AC1;
	GPIO_Init(GPIO_AC1, &GPIO_InitStructure);		

	GPIO_InitStructure.GPIO_Pin = PIN_AC2;
	GPIO_Init(GPIO_AC2, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = PIN_G1A;
	GPIO_Init(GPIO_G1A, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = PIN_G1B;
	GPIO_Init(GPIO_G1B, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = PIN_G2A;
	GPIO_Init(GPIO_G2A, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = PIN_G2B;
	GPIO_Init(GPIO_G2B, &GPIO_InitStructure);		

#if 0	
	DSO_SetDC(1, 0);	/* CH1ѡ��AC��� */
	DSO_SetDC(2, 0);	/* CH1ѡ��AC��� */
	DSO_SetGain(1, 0);	/* CH1ѡ��С���� ˥��1/5, ��2������1��ʾ��˥��1;1 */
	DSO_SetGain(2, 0);	/* CH2ѡ��С���� ˥��1/5, ��2������1��ʾ��˥��1;1 */	
#endif	
}

/*
*********************************************************************************************************
*	�� �� ��: DSO_SetDC
*	����˵��: ����AC DC���ģʽ
*	��    ��: _ch : ͨ��1��2
*			  _mode : 0��1.  1��ʾDC��� 0��ʾAC���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DSO_SetDC(uint8_t _ch, uint8_t _mode)
{	
#ifdef	D112_1
	if (_ch == 1)
	{
		GPIO_WriteBit(GPIO_AC1, PIN_AC1, (BitAction)_mode);
	}
	else
	{
		GPIO_WriteBit(GPIO_AC2, PIN_AC2, (BitAction)_mode);
	}
#else	/* ������ */
	if (_ch == 1)
	{
		if (_mode == 1)
		{
			AC1_1();
		}
		else
		{
			AC1_0();
		}
	}
	else
	{
		if (_mode == 1)
		{
			AC2_1();
		}
		else
		{
			AC2_0();
		}
	}	
#endif
	

}

/*
*********************************************************************************************************
*	�� �� ��: SetGainHigh
*	����˵��: ��������ģʽ
*	��    ��: _ch : ͨ��1��2
*			  _gain : 0��1.  1��ʾ1:1�� 0��ʾ˥��1/5
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DSO_SetGain(uint8_t _ch, uint8_t _gain)
{	
#ifdef	D112_1	/* �ɰ� */
	if (_ch == 1)
	{
		GPIO_WriteBit(GPIO_G1A, PIN_G1A, (BitAction)_gain);
	}
	else
	{
		GPIO_WriteBit(GPIO_G2A, PIN_G2A, (BitAction)_gain);
	}
#else		/* ������ D112-2 */
	if (_ch == 1)
	{
		if (_gain == 0)
		{
			G1A_0();
			G1B_0();
		}
		else if (_gain == 1)
		{
			G1A_1();
			G1B_0();
		}		
		else if (_gain == 2)
		{
			G1A_0();
			G1B_1();
		}
		else if (_gain == 3)
		{
			G1A_1();
			G1B_1();
		}			
	}
	else
	{
		if (_gain == 0)
		{
			G2A_0();
			G2B_0();
		}
		else if (_gain == 1)
		{
			G2A_1();
			G2B_0();
		}		
		else if (_gain == 2)
		{
			G2A_0();
			G2B_1();
		}
		else if (_gain == 3)
		{
			G2A_1();
			G2B_1();
		}	
	}
#endif	
}

/*
*********************************************************************************************************
*	�� �� ��: DSO_StartADC
*	����˵��: ��Ҫ���ģ����GPIO�����á�ADC�����á���ʱ���������Լ�DMA�����á�
*			- PC0/ADC123_10 �� CH1ͨ����ADC1��DMA2_Stream0
*			- PF10/ADC3_1N8 �� CH2ͨ��, ADC3��DMA2_Stream1
*			- ����ADC�����ڶ���ģʽ
*			- ������ͬ���ⲿ������ADC_ExternalTrigConv_T1_CC1
*			- TIM1��CC1Ƶ�ʵľ����˲���Ƶ��
*
*	��    ��: _uiBufAddr1 : DMAĿ���ַ��CH1���ݴ�ŵĻ�������ַ
*			  _uiBufAddr2 : DMAĿ���ַ��CH2���ݴ�ŵĻ�������ַ
*			  _uiCount : ���������������� (�����ֽ���)����ͨ��ͬ���ɼ�.
*			  _uiFreq : ����Ƶ�ʣ� Hz
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DSO_StartADC(uint32_t _uiBufAddr1, uint32_t _uiBufAddr2, uint32_t _uiCount, uint32_t _uiFreq)
{			
	/* ����GPIO. */
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_AHB1PeriphClockCmd(RCC_CH1 | RCC_CH2, ENABLE);
		
		/* ����ADC����Ϊģ������ģʽ ******************************/
		GPIO_InitStructure.GPIO_Pin = PIN_CH1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
		GPIO_Init(GPIO_CH1, &GPIO_InitStructure);

		/* ����ADC����Ϊģ������ģʽ ******************************/
		GPIO_InitStructure.GPIO_Pin = PIN_CH2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
		GPIO_Init(GPIO_CH2, &GPIO_InitStructure);		
	}

	/* DMA������ѡ����ֲ� 205ҳ �� 35. DMA1 ����ӳ�� + �� 36. DMA2 ����ӳ�� */ 
	{
		DMA_InitTypeDef       DMA_InitStructure;
		
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
		
		/* DMA2 Stream 0 channel 0 ��������ADC1 **************************************/
		DMA_DeInit(DMA2_Stream0);	/* �ڲ�����һЩ��־���������仰�������ٴ�����DMA�޷�Ӧ */
		DMA_InitStructure.DMA_Channel = DMA_Channel_0;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
		DMA_InitStructure.DMA_Memory0BaseAddr = _uiBufAddr1;	/* Ŀ���ַ */
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = _uiCount;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	// ����ѭ��ģʽ DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA2_Stream0, &DMA_InitStructure);
		DMA_Cmd(DMA2_Stream0, ENABLE);
	}

	{
		DMA_InitTypeDef       DMA_InitStructure;
		
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
		
		/* DMA2 Stream 1 channel 2 ��������ADC3 **************************************/
		DMA_DeInit(DMA2_Stream1);	/* �ڲ�����һЩ��־���������仰�������ٴ�����DMA�޷�Ӧ */
		DMA_InitStructure.DMA_Channel = DMA_Channel_2;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC3->DR;
		DMA_InitStructure.DMA_Memory0BaseAddr = _uiBufAddr2;	/* Ŀ���ַ */
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = _uiCount;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	// ����ѭ��ģʽ DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA2_Stream1, &DMA_InitStructure);
		
		DMA_Cmd(DMA2_Stream1, ENABLE);
	}
	
	/* ADC�������ֳ�ʼ��**********************************************************/
	{
		/*
		***************************************************************************
		  PCLK2 = HCLK / 2
		  ����ѡ�����2��Ƶ
		  ADCCLK = PCLK2 /2 = HCLK / 4 = 168 / 4 = 42M  [�ֲ����ʱ36M����ʱ�г�Ƶ]
		  ADC����Ƶ�ʣ� Sampling Time + Conversion Time = 3 + 12 cycles = 15cyc
						Conversion Time = 42MHz / 15cyc = 2.8Mbps.		
		****************************************************************************
		*/
		ADC_CommonInitTypeDef ADC_CommonInitStructure;

		ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //?
		ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
		ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
		ADC_CommonInit(&ADC_CommonInitStructure);
	}
	
	/* ���� PC0/ADC1_IN10  ******************************************************************/
	{
		ADC_InitTypeDef       ADC_InitStructure;
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
		
		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion = 1;

		/* ADC1 ����ͨ������ -------------------------------------------------------*/
		ADC_Init(ADC1, &ADC_InitStructure);
		ADC_RegularChannelConfig(ADC1, CHAN_CH1, 1, ADC_SampleTime_3Cycles);

		/* ʹ�� ADC1 DMA */
		ADC_DMACmd(ADC1, ENABLE);

		/* ʹ��DMA���� ���һ��������������ʹ��DMA, ����ѭ��ģʽ����ʹ�� -------*/
		//ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

		/* Enable ADC1 --------------------------------------------------------------*/
		ADC_Cmd(ADC1, ENABLE);
	}
	
	/* ���� PF10/ADC3_IN8  ******************************************************************/
	{
		ADC_InitTypeDef       ADC_InitStructure;
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
		
		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfConversion = 1;

		/* ADC1 ����ͨ������ -------------------------------------------------------*/
		ADC_Init(ADC3, &ADC_InitStructure);
		ADC_RegularChannelConfig(ADC3, CHAN_CH2, 1, ADC_SampleTime_3Cycles);

		/* ʹ�� ADC3 DMA */
		ADC_DMACmd(ADC3, ENABLE);

		/* ʹ��DMA���� ���һ��������������ʹ��DMA, ����ѭ��ģʽ����ʹ�� -------*/
		//ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

		/* Enable ADC3 --------------------------------------------------------------*/
		ADC_Cmd(ADC3, ENABLE);
	}

	/* ���ò���������ʱ����ʹ��TIM1 CC1 */
	DSO_SetSampRate(_uiFreq);	/* �޸Ĳ���Ƶ�ʣ�������TIM */
}

/*
*********************************************************************************************************
*	�� �� ��: DSO_StopADC
*	����˵��: �ر�ADC�������е����衣ADC, DMA, TIM
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DSO_StopADC(void)
{
	TIM_Cmd(TIM1, DISABLE);
	
	DMA_Cmd(DMA2_Stream1, DISABLE);
	DMA_Cmd(DMA2_Stream0, DISABLE);

	ADC_DMACmd(ADC1, DISABLE);
	ADC_Cmd(ADC1, DISABLE);	

	ADC_DMACmd(ADC3, DISABLE);
	ADC_Cmd(ADC3, DISABLE);		
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, DISABLE);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, DISABLE);	
}

/*
*********************************************************************************************************
*	�� �� ��: PauseADC
*	����˵��: ��ͣADC������׼���������ݡ���֤�´�DMA����������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DSO_PauseADC(void)
{
	TIM_Cmd(TIM1, DISABLE);

	ADC_DMACmd(ADC1, DISABLE);	/* ��仰�����У������޷������´�DMA */
	ADC_DMACmd(ADC3, DISABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: SetSampRate
*	����˵��: �޸Ĳ���Ƶ��
*	��    ��: freq : ����Ƶ�� ��λHz
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DSO_SetSampRate(uint32_t _ulFreq)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	/* ʹ��TIM1ʱ�� */

    TIM_Cmd(TIM1, DISABLE);
	TIM_SetCounter(TIM1, 0);
    /*
    ********************************************************************************
    system_stm32f4xx.c �ļ��� void SetSysClock(void) ������ʱ�ӵ��������£�

    HCLK = SYSCLK / 1     (AHB1Periph)
    PCLK2 = HCLK / 2      (APB2Periph)
    PCLK1 = HCLK / 4      (APB1Periph)

    ��ΪAPB1 prescaler != 1, ���� APB1�ϵ�TIMxCLK = PCLK1 x 2 = SystemCoreClock / 2;
    ��ΪAPB2 prescaler != 1, ���� APB2�ϵ�TIMxCLK = PCLK2 x 2 = SystemCoreClock;

    APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
    APB2 ��ʱ���� TIM1, TIM8 ,TIM9, TIM10, TIM11

    TIM1 ���������� = TIM1CLK / ��TIM_Period + 1��/��TIM_Prescaler + 1��
    ********************************************************************************
    */

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    /* ARR�Զ���װ�ؼĴ������ڵ�ֵ(��ʱʱ�䣩������Ƶ�ʺ���������»����ж�. */
	TIM_TimeBaseStructure.TIM_Period =  168000000 / (_ulFreq) - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;	/* ����Ƶ */
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0000;		/* TIM1 �� TIM8 �������� */		
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
    /**************ADC1 ADC3�Ĵ���***********************************************/
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = (TIM_TimeBaseStructure.TIM_Period + 1) / 2;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;		/* only for TIM1 and TIM8. �˴����������Ų�ͬ */	
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;			/* only for TIM1 and TIM8. */		 
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;		/* only for TIM1 and TIM8. */
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;		/* only for TIM1 and TIM8. */		
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);	/* ʹ��PWM ���, ���������GPIO */
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
