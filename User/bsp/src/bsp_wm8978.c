/*
*********************************************************************************************************
*
*	ģ������ : WM8978��ƵоƬ����ģ��
*	�ļ����� : bsp_wm8978.h
*	��    �� : V1.4
*	˵    �� : WM8978��ƵоƬ��STM32 I2S�ײ��������ڰ�����STM32-V5�������ϵ���ͨ����
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-02-01 armfly  ��ʽ����
*		V1.1    2013-06-12 armfly  �������Line ���벻�ܷ��������⡣�޸� wm8978_CfgAudioPath() ����
*		V1.2    2013-07-14 armfly  ��������Line����ӿ�����ĺ����� wm8978_SetLineGain()
*		V1.3    2015-10-18 armfly  ��ֲ��STM32F429���Ķ��ܴ�I2S�ӿ��޸�ΪSAI��Ƶ�ӿڡ�
*							-  wm8978_CfgAudioIF() �������ֳ��βΣ�����20bit
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"

/*
*********************************************************************************************************
*
*	��Ҫ��ʾ:
*	1��wm8978_ ��ͷ�ĺ����ǲ���WM8978�Ĵ���������WM8978�Ĵ�����ͨ��I2Cģ�����߽��е�
*	2��I2S_ ��ͷ�ĺ����ǲ���STM32  I2S��ؼĴ���
*	3��ʵ��¼�������Ӧ�ã���Ҫͬʱ����WM8978��STM32��I2S��
*	4�����ֺ����õ����βεĶ�����ST�̼����У����磺I2S_Standard_Phillips��I2S_Standard_MSB��I2S_Standard_LSB
*			  I2S_MCLKOutput_Enable��I2S_MCLKOutput_Disable
*			  I2S_AudioFreq_8K��I2S_AudioFreq_16K��I2S_AudioFreq_22K��I2S_AudioFreq_44K��I2S_AudioFreq_48
*			  I2S_Mode_MasterTx��I2S_Mode_MasterRx
*	5��ע���� pdf ָ���� wm8978.pdf �����ֲᣬwm8978de�Ĵ����ܶ࣬�õ��ļĴ�����ע��pdf�ļ���ҳ�룬���ڲ�ѯ
*
*********************************************************************************************************
*/

/* 
	������STM32-V6������---  SAI�ӿ� I2S���ߴ�����Ƶ���ݿ���	
		PF9/SAI1_FS_B
		PF8/SAI1_SCK_B
		PD6/SAI1_SD_A			ADC ¼��
		PF6/SAI1_SD_B			DAC ����
		PF7/SAI1_MCLK_B	
		

	STM32��SAI����Ϊ��ģʽ��SAIT_Block_A �� SAIT_Block_B ͬ��ģʽ����������SAIT_Block_B��Ϊ��ģ�����ʱ��.
	
	��ģ�� SAIT_Block_B �� SAI1_SD_B �������ڷ�������ģ�� SAIT_Block_A��SAI1_SD_A����¼����
	
	���ñ�׼I2SЭ�顣

    ��Ƶģ�������Ϊ��ڶ�����Ƶģ��ͬ��������������£�������λʱ�Ӻ�֡ͬ���źţ��Լ���ͨ��ʱռ���ⲿ���ŵ�����������Ϊ����һ��ģ��ͬ������Ƶģ�齫�ͷ��� SCK_x��
FS_x �� MCLK_x ���������� GPIO


*/

#define AUDIO_MAL_MODE_NORMAL
//#define AUDIO_MAL_MODE_CIRCULAR

/* For the DMA modes select the interrupt that will be used */

/* Uncomment this line to enable DMA Transfer Complete interrupt -- DMA��������ж� */
#define PLAY_DMA_IT_TC_EN    /* ������ */
#define REC_DMA_IT_TC_EN     /* ¼���� */

/* Uncomment this line to enable DMA Half Transfer Complete interrupt -- �봫���ж� */      
/* #define PLAY_DMA_IT_HT_EN */

/* Uncomment this line to enable DMA Transfer Error interrupt -- DMA��������ж� */  
/* #define PLAY_DMA_IT_TE_EN */  

#define  EVAL_AUDIO_IRQ_PREPRIO  3
#define  EVAL_AUDIO_IRQ_SUBRIO	 0
	
/*----------------------------------------------------------------------------
                    USER SAI defines parameters
 -----------------------------------------------------------------------------*/
/* In Main Application the PLL_SAI or PLL_I2S is configured to have this specific
   output, used to have a clean Audio Frequency */
#define SAI_ClockPLLSAI             ((uint32_t)11289600)

#define SAI_ClockPLLI2S             ((uint32_t)49152000)
#define SAI_ClockExtern             ((uint32_t)14000000)

//#define SAI_CLOCK_SOURCE            SAI_ClockPLLI2S       /* Default configuration */
#define SAI_CLOCK_SOURCE            SAI_ClockPLLSAI
/* #define SAI_CLOCK_SOURCE            SAI_ClockExtern */

/* ��Ƶ��ģ�����Ƶ���ݼĴ���, ����DMA�������� */
#define SAI_BLOCK1_DR	        (uint32_t)(&SAI1_Block_B->DR) 		/* ������ */
#define SAI_BLOCK2_DR	        (uint32_t)(&SAI1_Block_A->DR) 		/* ¼���� */

#define SAI_RCC                  RCC_APB2Periph_SAI1
#define SAI_GPIO_AF              GPIO_AF_SAI1
#define SAI_BLOCK1           	 SAI1_Block_B		/* ����Ƶ��ģ�� - ���� */
#define SAI_BLOCK2           	 SAI1_Block_A		/* ����Ƶ��ģ�� - ¼�� */

#define SAI_GPIO_RCC             (RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOD)

#define SAI_GPIO_FS              GPIOF
#define SAI_PIN_FS               GPIO_Pin_9
#define SAI_PINSRC_FS            GPIO_PinSource9

#define SAI_GPIO_SCK             GPIOF
#define SAI_PIN_SCK              GPIO_Pin_8
#define SAI_PINSRC_SCK           GPIO_PinSource8

#define SAI_GPIO_MCK             GPIOF
#define SAI_PIN_MCK              GPIO_Pin_7
#define SAI_PINSRC_MCK           GPIO_PinSource7

#define SAI_GPIO_SD1             GPIOF
#define SAI_PIN_SD1              GPIO_Pin_6
#define SAI_PINSRC_SD1           GPIO_PinSource6

#define SAI_GPIO_SD2             GPIOD
#define SAI_PIN_SD2              GPIO_Pin_6
#define SAI_PINSRC_SD2           GPIO_PinSource6


#define DMA_MAX_SZE                    0xFFFF
#define DMA_MAX(x)                (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)


/* DMA������صı�־:
	�봫��          HTIF  HTIE
	�������        TCIF  TCIE
	�������        TEIF  TEIE
	FIFO ����/����  FEIF  FEIE
	ֱ��ģʽ����    DMEIF  DMEI
*/

/* SAI DMA Stream definitions  <---- ������ SAI1_B ʹ�� DMA2_Stream5 _Channel_0 */
#define PLAY_DMA_CLOCK            RCC_AHB1Periph_DMA2
#define PLAY_DMA_STREAM           DMA2_Stream5
#define PLAY_DMA_CHANNEL          DMA_Channel_0
#define PLAY_DMA_IRQ              DMA2_Stream5_IRQn
#define PLAY_DMA_FLAG_TC          DMA_FLAG_TCIF5
#define PLAY_DMA_FLAG_HT          DMA_FLAG_HTIF5
#define PLAY_DMA_FLAG_FE          DMA_FLAG_FEIF5
#define PLAY_DMA_FLAG_TE          DMA_FLAG_TEIF5
#define PLAY_DMA_FLAG_DME         DMA_FLAG_DMEIF5
#define PLAY_DMA_FLAG_ALL         (uint32_t)(PLAY_DMA_FLAG_TC | PLAY_DMA_FLAG_HT | PLAY_DMA_FLAG_FE | PLAY_DMA_FLAG_TE | PLAY_DMA_FLAG_DME)
#define PLAY_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
#define PLAY_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord

#define PLAY_DMA_IRQHandler      DMA2_Stream5_IRQHandler

/* SAI DMA Stream definitions <---- ¼���� SAI1_A ʹ�� DMA2_Stream1 _Channel_0;  DMA2_Stream3��SDIOռ��  
	DMA2_Stream3 ��SDIOռ��
	DMA2_Stream1 ��CAMERAռ�ã���CAMER �� DMA Stream1 �޸�Ϊ DMA Stream7.
*/
#define REC_DMA_CLOCK            RCC_AHB1Periph_DMA2
#define REC_DMA_STREAM           DMA2_Stream1
#define REC_DMA_CHANNEL          DMA_Channel_0
#define REC_DMA_IRQ              DMA2_Stream1_IRQn
#define REC_DMA_FLAG_TC          DMA_FLAG_TCIF1
#define REC_DMA_FLAG_HT          DMA_FLAG_HTIF1
#define REC_DMA_FLAG_FE          DMA_FLAG_FEIF1
#define REC_DMA_FLAG_TE          DMA_FLAG_TEIF1
#define REC_DMA_FLAG_DME         DMA_FLAG_DMEIF1
#define REC_DMA_FLAG_ALL         (uint32_t)(REC_DMA_FLAG_TC | REC_DMA_FLAG_HT | REC_DMA_FLAG_FE | REC_DMA_FLAG_TE | REC_DMA_FLAG_DME)
#define REC_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
#define REC_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord

#define REC_DMA_IRQHandler       DMA2_Stream1_IRQHandler


/* ���ڱ�ģ���ڲ�ʹ�õľֲ����� */
static uint16_t wm8978_ReadReg(uint8_t _ucRegAddr);
static uint8_t wm8978_WriteReg(uint8_t _ucRegAddr, uint16_t _usValue);
static void SAI_GPIO_Config(void);

static void SAI_Mode_Config(uint8_t _mode, uint16_t _usStandard, uint32_t _uiWordLen, uint32_t _uiAudioFreq);

static void wm8978_Reset(void);

static void Play_DMA_Init(void);
static void Rec_DMA_Init(void);

static void PLLSAI_Config(void);
static void Rec_SAI_DMA_DeInit(void);
static void Play_SAI_DMA_DeInit(void);

/*
	wm8978�Ĵ�������
	����WM8978��I2C���߽ӿڲ�֧�ֶ�ȡ��������˼Ĵ���ֵ�������ڴ��У���д�Ĵ���ʱͬ�����»��棬���Ĵ���ʱ
	ֱ�ӷ��ػ����е�ֵ��
	�Ĵ���MAP ��WM8978.pdf �ĵ�67ҳ���Ĵ�����ַ��7bit�� �Ĵ���������9bit
*/
static uint16_t wm8978_RegCash[] = {
	0x000, 0x000, 0x000, 0x000, 0x050, 0x000, 0x140, 0x000,
	0x000, 0x000, 0x000, 0x0FF, 0x0FF, 0x000, 0x100, 0x0FF,
	0x0FF, 0x000, 0x12C, 0x02C, 0x02C, 0x02C, 0x02C, 0x000,
	0x032, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x038, 0x00B, 0x032, 0x000, 0x008, 0x00C, 0x093, 0x0E9,
	0x000, 0x000, 0x000, 0x000, 0x003, 0x010, 0x010, 0x100,
	0x100, 0x002, 0x001, 0x001, 0x039, 0x039, 0x039, 0x039,
	0x001, 0x001
};

/* ���ȫ�������ڳ�ʼ���������жϷ�������б�ʹ�ã��������Ϊȫ�ֱ��� */
static DMA_InitTypeDef DMA_Init_Play; 

static DMA_InitTypeDef DMA_Init_Rec; 

typedef struct
{
	/* This variable holds the total size of the audio file */
	uint32_t AudioTotalSize;
	/* This variable holds the remaining data in audio file */ 
	uint32_t AudioRemSize;
	/* This variable holds the current position of audio pointer */ 
	uint16_t *CurrentPos;  	
}AUDIO_CTRL_T;


static AUDIO_CTRL_T s_tPlay;
static AUDIO_CTRL_T s_tRec;


/*
*********************************************************************************************************
*	�� �� ��: wm8978_Init
*	����˵��: ����I2C GPIO�������I2C�����ϵ�WM8978�Ƿ�����
*	��    ��:  ��
*	�� �� ֵ: 1 ��ʾ��ʼ��������0��ʾ��ʼ��������
*********************************************************************************************************
*/
uint8_t wm8978_Init(void)
{
	uint8_t re;

	if (i2c_CheckDevice(WM8978_SLAVE_ADDRESS) == 0)	/* �������������STM32��GPIO�������ģ��I2Cʱ�� */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	wm8978_Reset();			/* Ӳ����λWM8978���мĴ�����ȱʡ״̬ */
	return re;
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_SetEarVolume
*	����˵��: �޸Ķ����������
*	��    ��:  _ucLeftVolume ������������ֵ, 0-63
*			  _ucLRightVolume : ����������ֵ,0-63
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_SetEarVolume(uint8_t _ucVolume)
{
	uint16_t regL;
	uint16_t regR;

	if (_ucVolume > 0x3F)
	{
		_ucVolume = 0x3F;
	}

	regL = _ucVolume;
	regR = _ucVolume;

	/*
		R52	LOUT1 Volume control
		R53	ROUT1 Volume control
	*/
	/* �ȸ�������������ֵ */
	wm8978_WriteReg(52, regL | 0x00);

	/* ��ͬ�������������������� */
	wm8978_WriteReg(53, regR | 0x100);	/* 0x180��ʾ ������Ϊ0ʱ�ٸ��£���������������ֵġ����ա��� */
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_SetSpkVolume
*	����˵��: �޸��������������
*	��    ��:  _ucLeftVolume ������������ֵ, 0-63
*			  _ucLRightVolume : ����������ֵ,0-63
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_SetSpkVolume(uint8_t _ucVolume)
{
	uint16_t regL;
	uint16_t regR;

	if (_ucVolume > 0x3F)
	{
		_ucVolume = 0x3F;
	}

	regL = _ucVolume;
	regR = _ucVolume;

	/*
		R54	LOUT2 (SPK) Volume control
		R55	ROUT2 (SPK) Volume control
	*/
	/* �ȸ�������������ֵ */
	wm8978_WriteReg(54, regL | 0x00);

	/* ��ͬ�������������������� */
	wm8978_WriteReg(55, regR | 0x100);	/* ������Ϊ0ʱ�ٸ��£���������������ֵġ����ա��� */
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_ReadEarVolume
*	����˵��: ���ض���ͨ��������.
*	��    ��:  ��
*	�� �� ֵ: ��ǰ����ֵ
*********************************************************************************************************
*/
uint8_t wm8978_ReadEarVolume(void)
{
	return (uint8_t)(wm8978_ReadReg(52) & 0x3F );
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_ReadSpkVolume
*	����˵��: ����������ͨ��������.
*	��    ��:  ��
*	�� �� ֵ: ��ǰ����ֵ
*********************************************************************************************************
*/
uint8_t wm8978_ReadSpkVolume(void)
{
	return (uint8_t)(wm8978_ReadReg(54) & 0x3F );
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_OutMute
*	����˵��: �������.
*	��    ��:  _ucMute ��1�Ǿ�����0�ǲ�����.
*	�� �� ֵ: ��ǰ����ֵ
*********************************************************************************************************
*/
void wm8978_OutMute(uint8_t _ucMute)
{
	uint16_t usRegValue;

	if (_ucMute == 1) /* ���� */
	{
		usRegValue = wm8978_ReadReg(52); /* Left Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(52, usRegValue);

		usRegValue = wm8978_ReadReg(53); /* Left Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(53, usRegValue);

		usRegValue = wm8978_ReadReg(54); /* Right Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(54, usRegValue);

		usRegValue = wm8978_ReadReg(55); /* Right Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(55, usRegValue);
	}
	else	/* ȡ������ */
	{
		usRegValue = wm8978_ReadReg(52);
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(52, usRegValue);

		usRegValue = wm8978_ReadReg(53); /* Left Mixer Control */
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(53, usRegValue);

		usRegValue = wm8978_ReadReg(54);
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(54, usRegValue);

		usRegValue = wm8978_ReadReg(55); /* Left Mixer Control */
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(55, usRegValue);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_SetMicGain
*	����˵��: ����MIC����
*	��    ��:  _ucGain ������ֵ, 0-63
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_SetMicGain(uint8_t _ucGain)
{
	if (_ucGain > GAIN_MAX)
	{
		_ucGain = GAIN_MAX;
	}

	/* PGA ��������  R45�� R46   pdf 19ҳ
		Bit8	INPPGAUPDATE
		Bit7	INPPGAZCL		�����ٸ���
		Bit6	INPPGAMUTEL		PGA����
		Bit5:0	����ֵ��010000��0dB
	*/
	wm8978_WriteReg(45, _ucGain);
	wm8978_WriteReg(46, _ucGain | (1 << 8));
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_SetLineGain
*	����˵��: ����Line����ͨ��������
*	��    ��:  _ucGain ������ֵ, 0-7. 7���0��С�� ��˥���ɷŴ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_SetLineGain(uint8_t _ucGain)
{
	uint16_t usRegValue;

	if (_ucGain > 7)
	{
		_ucGain = 7;
	}

	/*
		Mic �����ŵ��������� PGABOOSTL �� PGABOOSTR ����
		Aux �����ŵ������������� AUXL2BOOSTVO[2:0] �� AUXR2BOOSTVO[2:0] ����
		Line �����ŵ��������� LIP2BOOSTVOL[2:0] �� RIP2BOOSTVOL[2:0] ����
	*/
	/*	pdf 21ҳ��R47������������R48����������, MIC ������ƼĴ���
		R47 (R48���������ͬ)
		B8		PGABOOSTL	= 1,   0��ʾMIC�ź�ֱͨ�����棬1��ʾMIC�ź�+20dB���棨ͨ���Ծٵ�·��
		B7		= 0�� ����
		B6:4	L2_2BOOSTVOL = x�� 0��ʾ��ֹ��1-7��ʾ����-12dB ~ +6dB  ������˥��Ҳ���ԷŴ�
		B3		= 0�� ����
		B2:0`	AUXL2BOOSTVOL = x��0��ʾ��ֹ��1-7��ʾ����-12dB ~ +6dB  ������˥��Ҳ���ԷŴ�
	*/

	usRegValue = wm8978_ReadReg(47);
	usRegValue &= 0x8F;/* ��Bit6:4��0   1000 1111*/
	usRegValue |= (_ucGain << 4);
	wm8978_WriteReg(47, usRegValue);	/* д����������������ƼĴ��� */

	usRegValue = wm8978_ReadReg(48);
	usRegValue &= 0x8F;/* ��Bit6:4��0   1000 1111*/
	usRegValue |= (_ucGain << 4);
	wm8978_WriteReg(48, usRegValue);	/* д����������������ƼĴ��� */
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_PowerDown
*	����˵��: �ر�wm8978������͹���ģʽ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_PowerDown(void)
{
	wm8978_Reset();			/* Ӳ����λWM8978���мĴ�����ȱʡ״̬ */
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_CfgAudioIF
*	����˵��: ����WM8978����Ƶ�ӿ�(I2S)
*	��    ��:
*			  _usStandard : �ӿڱ�׼��I2S_Standard_Phillips, I2S_Standard_MSB �� I2S_Standard_LSB
*			  _ucWordLen : �ֳ���16��24��32��20bit��ʽ��
*			  _usMode : CPU I2S�Ĺ���ģʽ��I2S_Mode_MasterTx��I2S_Mode_MasterRx��
*						������������Ӳ����֧�� I2S_Mode_SlaveTx��I2S_Mode_SlaveRx ģʽ������ҪWM8978����
*						�ⲿ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_CfgAudioIF(uint16_t _usStandard, uint8_t _ucWordLen)
{
	uint16_t usReg;

	/* pdf 67ҳ���Ĵ����б� */

	/*	REG R4, ��Ƶ�ӿڿ��ƼĴ���
		B8		BCP	 = X, BCLK���ԣ�0��ʾ������1��ʾ����
		B7		LRCP = x, LRCʱ�Ӽ��ԣ�0��ʾ������1��ʾ����
		B6:5	WL = x�� �ֳ���00=16bit��01=20bit��10=24bit��11=32bit ���Ҷ���ģʽֻ�ܲ��������24bit)
		B4:3	FMT = x����Ƶ���ݸ�ʽ��00=�Ҷ��룬01=����룬10=I2S��ʽ��11=PCM
		B2		DACLRSWAP = x, ����DAC���ݳ�����LRCʱ�ӵ���߻����ұ�
		B1 		ADCLRSWAP = x������ADC���ݳ�����LRCʱ�ӵ���߻����ұ�
		B0		MONO	= 0��0��ʾ��������1��ʾ������������������Ч
	*/
	usReg = 0;
	if (_usStandard == I2S_Standard_Phillips)	/* I2S�����ֱ�׼ */
	{
		usReg |= (2 << 3);
	}
	else if (_usStandard == I2S_Standard_MSB)	/* MSB�����׼(�����) */
	{
		usReg |= (1 << 3);
	}
	else if (_usStandard == I2S_Standard_LSB)	/* LSB�����׼(�Ҷ���) */
	{
		usReg |= (0 << 3);
	}
	else	/* PCM��׼(16λͨ��֡�ϴ������֡ͬ������16λ����֡��չΪ32λͨ��֡) */
	{
		usReg |= (3 << 3);;
	}

	if (_ucWordLen == 24)
	{
		usReg |= (2 << 5);
	}
	else if (_ucWordLen == 32)
	{
		usReg |= (3 << 5);
	}
	else if (_ucWordLen == 20)
	{
		usReg |= (1 << 5);
	}	
	else
	{
		usReg |= (0 << 5);		/* 16bit */
	}
	wm8978_WriteReg(4, usReg);

	/* R5  pdf 57ҳ */


	/*
		R6��ʱ�Ӳ������ƼĴ���
		MS = 0,  WM8978����ʱ�ӣ���MCU�ṩMCLKʱ��
	*/
	wm8978_WriteReg(6, 0x000);
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_ReadReg
*	����˵��: ��cash�ж��ض���wm8978�Ĵ���
*	��    ��:  _ucRegAddr �� �Ĵ�����ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint16_t wm8978_ReadReg(uint8_t _ucRegAddr)
{
	return wm8978_RegCash[_ucRegAddr];
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_WriteReg
*	����˵��: дwm8978�Ĵ���
*	��    ��:  _ucRegAddr �� �Ĵ�����ַ
*			  _usValue ���Ĵ���ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t wm8978_WriteReg(uint8_t _ucRegAddr, uint16_t _usValue)
{
	uint8_t ucAck;

	/* ������ʼλ */
	i2c_Start();

	/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
	i2c_SendByte(WM8978_SLAVE_ADDRESS | I2C_WR);

	/* ���ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* ���Ϳ����ֽ�1 */
	i2c_SendByte(((_ucRegAddr << 1) & 0xFE) | ((_usValue >> 8) & 0x1));

	/* ���ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* ���Ϳ����ֽ�2 */
	i2c_SendByte(_usValue & 0xFF);

	/* ���ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* ����STOP */
	i2c_Stop();

	wm8978_RegCash[_ucRegAddr] = _usValue;
	return 1;
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_CfgInOut
*	����˵��: ����wm8978��Ƶͨ��
*	��    ��:
*			_InPath : ��Ƶ����ͨ������
*			_OutPath : ��Ƶ���ͨ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_CfgAudioPath(uint16_t _InPath, uint16_t _OutPath)
{
	uint16_t usReg;

	/* �鿴WM8978�����ֲ�� REGISTER MAP �½ڣ� ��67ҳ */

	if ((_InPath == IN_PATH_OFF) && (_OutPath == OUT_PATH_OFF))
	{
		wm8978_PowerDown();
		return;
	}

	/* --------------------------- ��1������������ͨ���������üĴ��� -----------------------*/
	/*
		R1 �Ĵ��� Power manage 1
		Bit8    BUFDCOPEN,  Output stage 1.5xAVDD/2 driver enable
 		Bit7    OUT4MIXEN, OUT4 mixer enable
		Bit6    OUT3MIXEN, OUT3 mixer enable
		Bit5    PLLEN	.����
		Bit4    MICBEN	,Microphone Bias Enable (MICƫ�õ�·ʹ��)
		Bit3    BIASEN	,Analogue amplifier bias control ��������Ϊ1ģ��Ŵ����Ź���
		Bit2    BUFIOEN , Unused input/output tie off buffer enable
		Bit1:0  VMIDSEL, ��������Ϊ��00ֵģ��Ŵ����Ź���
	*/
	usReg = (1 << 3) | (3 << 0);
	if (_OutPath & OUT3_4_ON) 	/* OUT3��OUT4ʹ�������GSMģ�� */
	{
		usReg |= ((1 << 7) | (1 << 6));
	}
	if ((_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= (1 << 4);
	}
	wm8978_WriteReg(1, usReg);	/* д�Ĵ��� */

	/*
		R2 �Ĵ��� Power manage 2
		Bit8	ROUT1EN,	ROUT1 output enable �������������ʹ��
		Bit7	LOUT1EN,	LOUT1 output enable �������������ʹ��
		Bit6	SLEEP, 		0 = Normal device operation   1 = Residual current reduced in device standby mode
		Bit5	BOOSTENR,	Right channel Input BOOST enable ����ͨ���Ծٵ�·ʹ��. �õ�PGA�Ŵ���ʱ����ʹ��
		Bit4	BOOSTENL,	Left channel Input BOOST enable
		Bit3	INPGAENR,	Right channel input PGA enable ����������PGAʹ��
		Bit2	INPGAENL,	Left channel input PGA enable
		Bit1	ADCENR,		Enable ADC right channel
		Bit0	ADCENL,		Enable ADC left channel
	*/
	usReg = 0;
	if (_OutPath & EAR_LEFT_ON)
	{
		usReg |= (1 << 7);
	}
	if (_OutPath & EAR_RIGHT_ON)
	{
		usReg |= (1 << 8);
	}
	if (_InPath & MIC_LEFT_ON)
	{
		usReg |= ((1 << 4) | (1 << 2));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 3));
	}
	if (_InPath & LINE_ON)
	{
		usReg |= ((1 << 4) | (1 << 5));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 3));
	}
	if (_InPath & ADC_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(2, usReg);	/* д�Ĵ��� */

	/*
		R3 �Ĵ��� Power manage 3
		Bit8	OUT4EN,		OUT4 enable
		Bit7	OUT3EN,		OUT3 enable
		Bit6	LOUT2EN,	LOUT2 output enable
		Bit5	ROUT2EN,	ROUT2 output enable
		Bit4	0,
		Bit3	RMIXEN,		Right mixer enable
		Bit2	LMIXEN,		Left mixer enable
		Bit1	DACENR,		Right channel DAC enable
		Bit0	DACENL,		Left channel DAC enable
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= ((1 << 8) | (1 << 7));
	}
	if (_OutPath & SPK_ON)
	{
		usReg |= ((1 << 6) | (1 << 5));
	}
	if (_OutPath != OUT_PATH_OFF)
	{
		usReg |= ((1 << 3) | (1 << 2));
	}
	if (_InPath & DAC_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(3, usReg);	/* д�Ĵ��� */

	/*
		R44 �Ĵ��� Input ctrl

		Bit8	MBVSEL, 		Microphone Bias Voltage Control   0 = 0.9 * AVDD   1 = 0.6 * AVDD
		Bit7	0
		Bit6	R2_2INPPGA,		Connect R2 pin to right channel input PGA positive terminal
		Bit5	RIN2INPPGA,		Connect RIN pin to right channel input PGA negative terminal
		Bit4	RIP2INPPGA,		Connect RIP pin to right channel input PGA amplifier positive terminal
		Bit3	0
		Bit2	L2_2INPPGA,		Connect L2 pin to left channel input PGA positive terminal
		Bit1	LIN2INPPGA,		Connect LIN pin to left channel input PGA negative terminal
		Bit0	LIP2INPPGA,		Connect LIP pin to left channel input PGA amplifier positive terminal
	*/
	usReg = 0 << 8;
	if (_InPath & LINE_ON)
	{
		usReg |= ((1 << 6) | (1 << 2));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 4));
	}
	if (_InPath & MIC_LEFT_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(44, usReg);	/* д�Ĵ��� */


	/*
		R14 �Ĵ��� ADC Control
		���ø�ͨ�˲�������ѡ�ģ� pdf 24��25ҳ,
		Bit8 	HPFEN,	High Pass Filter Enable��ͨ�˲���ʹ�ܣ�0��ʾ��ֹ��1��ʾʹ��
		BIt7 	HPFAPP,	Select audio mode or application mode ѡ����Ƶģʽ��Ӧ��ģʽ��0��ʾ��Ƶģʽ��
		Bit6:4	HPFCUT��Application mode cut-off frequency  000-111ѡ��Ӧ��ģʽ�Ľ�ֹƵ��
		Bit3 	ADCOSR,	ADC oversample rate select: 0=64x (lower power) 1=128x (best performance)
		Bit2   	0
		Bit1 	ADC right channel polarity adjust:  0=normal  1=inverted
		Bit0 	ADC left channel polarity adjust:  0=normal 1=inverted
	*/
	if (_InPath & ADC_ON)
	{
		usReg = (1 << 3) | (0 << 8) | (4 << 0);		/* ��ֹADC��ͨ�˲���, ���ý���Ƶ�� */
	}
	else
	{
		usReg = 0;
	}
	wm8978_WriteReg(14, usReg);	/* д�Ĵ��� */

	/* �����ݲ��˲�����notch filter������Ҫ�������ƻ�Ͳ����������������Х��.  ��ʱ�ر�
		R27��R28��R29��R30 ���ڿ����޲��˲�����pdf 26ҳ
		R7�� Bit7 NFEN = 0 ��ʾ��ֹ��1��ʾʹ��
	*/
	if (_InPath & ADC_ON)
	{
		usReg = (0 << 7);
		wm8978_WriteReg(27, usReg);	/* д�Ĵ��� */
		usReg = 0;
		wm8978_WriteReg(28, usReg);	/* д�Ĵ���,��0����Ϊ�Ѿ���ֹ������Ҳ�ɲ��� */
		wm8978_WriteReg(29, usReg);	/* д�Ĵ���,��0����Ϊ�Ѿ���ֹ������Ҳ�ɲ��� */
		wm8978_WriteReg(30, usReg);	/* д�Ĵ���,��0����Ϊ�Ѿ���ֹ������Ҳ�ɲ��� */
	}

	/* �Զ�������� ALC, R32  - 34  pdf 19ҳ */
	{
		usReg = 0;		/* ��ֹ�Զ�������� */
		wm8978_WriteReg(32, usReg);
		wm8978_WriteReg(33, usReg);
		wm8978_WriteReg(34, usReg);
	}

	/*  R35  ALC Noise Gate Control
		Bit3	NGATEN, Noise gate function enable
		Bit2:0	Noise gate threshold:
	*/
	usReg = (3 << 1) | (7 << 0);		/* ��ֹ�Զ�������� */
	wm8978_WriteReg(35, usReg);

	/*
		Mic �����ŵ��������� PGABOOSTL �� PGABOOSTR ����
		Aux �����ŵ������������� AUXL2BOOSTVO[2:0] �� AUXR2BOOSTVO[2:0] ����
		Line �����ŵ��������� LIP2BOOSTVOL[2:0] �� RIP2BOOSTVOL[2:0] ����
	*/
	/*	pdf 21ҳ��R47������������R48����������, MIC ������ƼĴ���
		R47 (R48���������ͬ)
		B8		PGABOOSTL	= 1,   0��ʾMIC�ź�ֱͨ�����棬1��ʾMIC�ź�+20dB���棨ͨ���Ծٵ�·��
		B7		= 0�� ����
		B6:4	L2_2BOOSTVOL = x�� 0��ʾ��ֹ��1-7��ʾ����-12dB ~ +6dB  ������˥��Ҳ���ԷŴ�
		B3		= 0�� ����
		B2:0`	AUXL2BOOSTVOL = x��0��ʾ��ֹ��1-7��ʾ����-12dB ~ +6dB  ������˥��Ҳ���ԷŴ�
	*/
	usReg = 0;
	if ((_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= (1 << 8);	/* MIC����ȡ+20dB */
	}
	if (_InPath & AUX_ON)
	{
		usReg |= (3 << 0);	/* Aux����̶�ȡ3���û��������е��� */
	}
	if (_InPath & LINE_ON)
	{
		usReg |= (3 << 4);	/* Line����̶�ȡ3���û��������е��� */
	}
	wm8978_WriteReg(47, usReg);	/* д����������������ƼĴ��� */
	wm8978_WriteReg(48, usReg);	/* д����������������ƼĴ��� */

	/* ����ADC�������ƣ�pdf 27ҳ
		R15 ����������ADC������R16����������ADC����
		Bit8 	ADCVU  = 1 ʱ�Ÿ��£�����ͬ����������������ADC����
		Bit7:0 	����ѡ�� 0000 0000 = ����
						   0000 0001 = -127dB
						   0000 0010 = -12.5dB  ��0.5dB ������
						   1111 1111 = 0dB  ����˥����
	*/
	usReg = 0xFF;
	wm8978_WriteReg(15, usReg);	/* ѡ��0dB���Ȼ��������� */
	usReg = 0x1FF;
	wm8978_WriteReg(16, usReg);	/* ͬ�������������� */

	/* ͨ�� wm8978_SetMicGain ��������mic PGA���� */

	/*	R43 �Ĵ���  AUXR �C ROUT2 BEEP Mixer Function
		B8:6 = 0

		B5	 MUTERPGA2INV,	Mute input to INVROUT2 mixer
		B4	 INVROUT2,  Invert ROUT2 output �����������������
		B3:1 BEEPVOL = 7;	AUXR input to ROUT2 inverter gain
		B0	 BEEPEN = 1;	Enable AUXR beep input

	*/
	usReg = 0;
	if (_OutPath & SPK_ON)
	{
		usReg |= (1 << 4);	/* ROUT2 ����, �������������� */
	}
	if (_InPath & AUX_ON)
	{
		usReg |= ((7 << 1) | (1 << 0));
	}
	wm8978_WriteReg(43, usReg);

	/* R49  Output ctrl
		B8:7	0
		B6		DACL2RMIX,	Left DAC output to right output mixer
		B5		DACR2LMIX,	Right DAC output to left output
		B4		OUT4BOOST,	0 = OUT4 output gain = -1; DC = AVDD / 2��1 = OUT4 output gain = +1.5��DC = 1.5 x AVDD / 2
		B3		OUT3BOOST,	0 = OUT3 output gain = -1; DC = AVDD / 2��1 = OUT3 output gain = +1.5��DC = 1.5 x AVDD / 2
		B2		SPKBOOST,	0 = Speaker gain = -1;  DC = AVDD / 2 ; 1 = Speaker gain = +1.5; DC = 1.5 x AVDD / 2
		B1		TSDEN,   Thermal Shutdown Enable  �������ȱ���ʹ�ܣ�ȱʡ1��
		B0		VROI,	Disabled Outputs to VREF Resistance
	*/
	usReg = 0;
	if (_InPath & DAC_ON)
	{
		usReg |= ((1 << 6) | (1 << 5));
	}
	if (_OutPath & SPK_ON)
	{
		usReg |=  ((1 << 2) | (1 << 1));	/* SPK 1.5x����,  �ȱ���ʹ�� */
	}
	if (_OutPath & OUT3_4_ON)
	{
		usReg |=  ((1 << 4) | (1 << 3));	/* BOOT3  BOOT4  1.5x���� */
	}
	wm8978_WriteReg(49, usReg);

	/*	REG 50    (50����������51�������������üĴ�������һ��) pdf 40ҳ
		B8:6	AUXLMIXVOL = 111	AUX����FM�������ź�����
		B5		AUXL2LMIX = 1		Left Auxilliary input to left channel
		B4:2	BYPLMIXVOL			����
		B1		BYPL2LMIX = 0;		Left bypass path (from the left channel input boost output) to left output mixer
		B0		DACL2LMIX = 1;		Left DAC output to left output mixer
	*/
	usReg = 0;
	if (_InPath & AUX_ON)
	{
		usReg |= ((7 << 6) | (1 << 5));
	}
	if ((_InPath & LINE_ON) || (_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= ((7 << 2) | (1 << 1));
	}
	if (_InPath & DAC_ON)
	{
		usReg |= (1 << 0);
	}
	wm8978_WriteReg(50, usReg);
	wm8978_WriteReg(51, usReg);

	/*	R56 �Ĵ���   OUT3 mixer ctrl
		B8:7	0
		B6		OUT3MUTE,  	0 = Output stage outputs OUT3 mixer;  1 = Output stage muted �C drives out VMID.
		B5:4	0
		B3		BYPL2OUT3,	OUT4 mixer output to OUT3  (����)
		B4		0
		B2		LMIX2OUT3,	Left ADC input to OUT3
		B1		LDAC2OUT3,	Left DAC mixer to OUT3
		B0		LDAC2OUT3,	Left DAC output to OUT3
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= (1 << 3);
	}
	wm8978_WriteReg(56, usReg);

	/* R57 �Ĵ���		OUT4 (MONO) mixer ctrl
		B8:7	0
		B6		OUT4MUTE,	0 = Output stage outputs OUT4 mixer  1 = Output stage muted �C drives outVMID.
		B5		HALFSIG,	0 = OUT4 normal output	1 = OUT4 attenuated by 6dB
		B4		LMIX2OUT4,	Left DAC mixer to OUT4
		B3		LDAC2UT4,	Left DAC to OUT4
		B2		BYPR2OUT4,	Right ADC input to OUT4
		B1		RMIX2OUT4,	Right DAC mixer to OUT4
		B0		RDAC2OUT4,	Right DAC output to OUT4
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= ((1 << 4) |  (1 << 1));
	}
	wm8978_WriteReg(57, usReg);


	/* R11, 12 �Ĵ��� DAC��������
		R11		Left DAC Digital Volume
		R12		Right DAC Digital Volume
	*/
	if (_InPath & DAC_ON)
	{
		wm8978_WriteReg(11, 255);
		wm8978_WriteReg(12, 255 | 0x100);
	}
	else
	{
		wm8978_WriteReg(11, 0);
		wm8978_WriteReg(12, 0 | 0x100);
	}

	/*	R10 �Ĵ��� DAC Control
		B8	0
		B7	0
		B6	SOFTMUTE,	Softmute enable:
		B5	0
		B4	0
		B3	DACOSR128,	DAC oversampling rate: 0=64x (lowest power) 1=128x (best performance)
		B2	AMUTE,		Automute enable
		B1	DACPOLR,	Right DAC output polarity
		B0	DACPOLL,	Left DAC output polarity:
	*/
	if (_InPath & DAC_ON)
	{
		wm8978_WriteReg(10, 0);
	}
	;
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_NotchFilter
*	����˵��: �����ݲ��˲�����notch filter������Ҫ�������ƻ�Ͳ����������������Х��
*	��    ��:  NFA0[13:0] and NFA1[13:0]
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_NotchFilter(uint16_t _NFA0, uint16_t _NFA1)
{
	uint16_t usReg;

	/*  page 26
		A programmable notch filter is provided. This filter has a variable centre frequency and bandwidth,
		programmable via two coefficients, a0 and a1. a0 and a1 are represented by the register bits
		NFA0[13:0] and NFA1[13:0]. Because these coefficient values require four register writes to setup
		there is an NFU (Notch Filter Update) flag which should be set only when all four registers are setup.
	*/
	usReg = (1 << 7) | (_NFA0 & 0x3F);
	wm8978_WriteReg(27, usReg);	/* д�Ĵ��� */

	usReg = ((_NFA0 >> 7) & 0x3F);
	wm8978_WriteReg(28, usReg);	/* д�Ĵ��� */

	usReg = (_NFA1 & 0x3F);
	wm8978_WriteReg(29, usReg);	/* д�Ĵ��� */

	usReg = (1 << 8) | ((_NFA1 >> 7) & 0x3F);
	wm8978_WriteReg(30, usReg);	/* д�Ĵ��� */
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_CtrlGPIO1
*	����˵��: ����WM8978��GPIO1�������0��1
*	��    ��:  _ucValue ��GPIO1���ֵ��0��1
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void wm8978_CtrlGPIO1(uint8_t _ucValue)
{
	uint16_t usRegValue;

	/* R8�� pdf 62ҳ */
	if (_ucValue == 0) /* ���0 */
	{
		usRegValue = 6; /* B2:0 = 110 */
	}
	else
	{
		usRegValue = 7; /* B2:0 = 111 */
	}
	wm8978_WriteReg(8, usRegValue);
}

/*
*********************************************************************************************************
*	�� �� ��: wm8978_Reset
*	����˵��: ��λwm8978�����еļĴ���ֵ�ָ���ȱʡֵ
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void wm8978_Reset(void)
{
	/* wm8978�Ĵ���ȱʡֵ */
	const uint16_t reg_default[] = {
	0x000, 0x000, 0x000, 0x000, 0x050, 0x000, 0x140, 0x000,
	0x000, 0x000, 0x000, 0x0FF, 0x0FF, 0x000, 0x100, 0x0FF,
	0x0FF, 0x000, 0x12C, 0x02C, 0x02C, 0x02C, 0x02C, 0x000,
	0x032, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x038, 0x00B, 0x032, 0x000, 0x008, 0x00C, 0x093, 0x0E9,
	0x000, 0x000, 0x000, 0x000, 0x003, 0x010, 0x010, 0x100,
	0x100, 0x002, 0x001, 0x001, 0x039, 0x039, 0x039, 0x039,
	0x001, 0x001
	};
	uint8_t i;

	wm8978_WriteReg(0x00, 0);

	for (i = 0; i < sizeof(reg_default) / 2; i++)
	{
		wm8978_RegCash[i] = reg_default[i];
	}
}

/*
*********************************************************************************************************
*	                     ����Ĵ����Ǻ�STM32 SAI��Ƶ�ӿ�Ӳ����ص�
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	�� �� ��: I2S_CODEC_Init
*	����˵��: ����GPIO���ź��ж�ͨ������codecӦ��
*	��    ��: _ucMode : 1 ��ʾ������2��ʾ¼���� 3��ʾ��¼�߷�(��δ֧��)
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AUDIO_Init(uint8_t _ucMode, uint16_t _usStandard, uint32_t _uiWordLen, uint32_t _uiAudioFreq)
{	
	PLLSAI_Config();		/* ����SAI��ʱ��Դ */
			
	SAI_GPIO_Config();		/* ����SAI��ص� GPIO���� */
	
	Play_SAI_DMA_DeInit();	/* ��ֹ����ͨ����DMA���� */	
	
	Rec_SAI_DMA_DeInit();	/* ��ֹ¼��ͨ����DMA���� */
	
	
	/* ��ֹSAIʱ�� -> ����ʹ��SAIʱ�� */
	RCC_APB2PeriphClockCmd(SAI_RCC, DISABLE);
	RCC_APB2PeriphClockCmd(SAI_RCC, ENABLE);
	
	SAI_Cmd(SAI_BLOCK1, DISABLE); 		
	SAI_Cmd(SAI_BLOCK2, DISABLE);  

	SAI_Mode_Config(_ucMode, _usStandard,  _uiWordLen, _uiAudioFreq);

	if (_ucMode == 1)	/* ���� */
	{
		Play_DMA_Init();
		
		//SAI_Cmd(SAI_BLOCK1, ENABLE); 
		//SAI_SendData(SAI1_Block_B, 0x00);	
		
	}
	else if (_ucMode == 2)	/* ¼�� */
	{
		Rec_DMA_Init();
		
		SAI_Cmd(SAI_BLOCK2, ENABLE); 
		SAI_ReceiveData(SAI1_Block_A);
	}
	else
	{
		Play_DMA_Init();
		Rec_DMA_Init();

		SAI_Cmd(SAI_BLOCK1, ENABLE); 		
		SAI_Cmd(SAI_BLOCK2, ENABLE); 
	}
}


/**
  * @brief  Starts playing audio stream from a data buffer for a determined size. 
  * @param  pBuffer: Pointer to the buffer 
  * @param  Size: Number of audio data BYTES.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t AUDIO_Play(uint16_t* pBuffer, uint32_t Size)
{		
	/* Update the Media layer and enable it for play */  
	{
		/* Configure the buffer address and size */
		DMA_Init_Play.DMA_Memory0BaseAddr = (uint32_t)pBuffer;
		DMA_Init_Play.DMA_BufferSize = (uint32_t)(DMA_MAX(s_tPlay.AudioTotalSize / 2));
		
		/* Configure the DMA Stream with the new parameters */
		DMA_Init(PLAY_DMA_STREAM, &DMA_Init_Play);
		
		/* Enable the SAI DMA Stream*/
		DMA_Cmd(PLAY_DMA_STREAM, ENABLE);
		
		/* If the SAI peripheral is still not enabled, enable it */
		if (SAI_GetCmdStatus(SAI_BLOCK1) == DISABLE)
		{
			SAI_Cmd(SAI_BLOCK1, ENABLE);
		} 		
	}

	/* Set the total number of data to be played (count in half-word) */
	s_tPlay.AudioTotalSize = Size/2;
	
	/* Update the remaining number of data to be played */
	s_tPlay.AudioRemSize = (Size/2) - DMA_MAX(s_tPlay.AudioTotalSize);
	
	/* Update the current audio pointer position */
	s_tPlay.CurrentPos = pBuffer + DMA_MAX(s_tPlay.AudioTotalSize);
	
	return 0;
}


/**
  * @brief  Starts playing audio stream from a data buffer for a determined size. 
  * @param  pBuffer: Pointer to the buffer 
  * @param  Size: Number of audio data BYTES.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t AUDIO_Record(uint16_t* pBuffer, uint32_t Size)
{	
	/* Update the Media layer and enable it for play */  
	{
		/* Configure the buffer address and size */
		DMA_Init_Rec.DMA_Memory0BaseAddr = (uint32_t)pBuffer;
		DMA_Init_Rec.DMA_BufferSize = (uint32_t)(DMA_MAX(s_tRec.AudioTotalSize / 2));
		
		/* Configure the DMA Stream with the new parameters */
		DMA_Init(REC_DMA_STREAM, &DMA_Init_Rec);
		
		/* Enable the SAI DMA Stream*/
		DMA_Cmd(REC_DMA_STREAM, ENABLE);
		
		/* If the SAI peripheral is still not enabled, enable it */
		if (SAI_GetCmdStatus(SAI_BLOCK2) == DISABLE)
		{
			SAI_Cmd(SAI_BLOCK2, ENABLE);
		} 		
	}

	/* Set the total number of data to be played (count in half-word) */
	s_tRec.AudioTotalSize = Size/2;	
	
	/* Update the remaining number of data to be played */
	s_tRec.AudioRemSize = (Size/2) - DMA_MAX(s_tRec.AudioTotalSize);
	
	/* Update the current audio pointer position */
	s_tRec.CurrentPos = pBuffer + DMA_MAX(s_tRec.AudioTotalSize);
	
	return 0;
}

/**
  * @brief  This function Pauses or Resumes the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used. In all cases the SAI 
  *         peripheral is disabled. 
  * 
  * @WARNING When calling EVAL_AUDIO_PauseResume() function for pause, only
  *          this function should be called for resume (use of EVAL_AUDIO_Play() 
  *          function for resume could lead to unexpected behavior).
  * 
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @retval 0 if correct communication, else wrong communication
  */
void AUDIO_Pause(void)
{    
	/* Disable the SAI DMA request */
	SAI_DMACmd(SAI_BLOCK1, DISABLE);
	
	/* Pause the SAI DMA Stream 
	Note. For the STM32F4xx devices, the DMA implements a pause feature, 
	      by disabling the stream, all configuration is preserved and data 
	      transfer is paused till the next enable of the stream.*/
	DMA_Cmd(PLAY_DMA_STREAM, DISABLE);
}

/**
  * @brief  This function Pauses or Resumes the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used. In all cases the SAI 
  *         peripheral is disabled. 
  * 
  * @WARNING When calling EVAL_AUDIO_PauseResume() function for pause, only
  *          this function should be called for resume (use of EVAL_AUDIO_Play() 
  *          function for resume could lead to unexpected behavior).
  * 
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @retval 0 if correct communication, else wrong communication
  */
void AUDIO_Resume(uint32_t Cmd)
{    
	/* Enable the I2S DMA request */
	SAI_DMACmd(SAI_BLOCK1, ENABLE);
	
	/* Resume the SAI DMA Stream 
	Note. For the STM32F4xx devices, the DMA implements a pause feature, 
	      by disabling the stream, all configuration is preserved and data 
	      transfer is paused till the next enable of the stream.*/
	DMA_Cmd(PLAY_DMA_STREAM, ENABLE);
	
	/* If the SAI peripheral is still not enabled, enable it */
	if (SAI_GetCmdStatus(SAI_BLOCK1) == DISABLE)
	{
		SAI_Cmd(SAI_BLOCK1, ENABLE);
	}   
}

/**
  * @brief  Stops audio playing and enables the MUTE mode by software. 
  * @param  Option: could be one of the following parameters 
  *           - CODEC_PDWN_SW: To enable the MUTE mode.                   
  *           - CODEC_PDWN_HW: To reset the codec by writing in 0x0000 address.
  *                            All registers will be reset to their default state. 
  *                            Then need to reconfigure the Codec after that.  
  * @note   Codec will not be physically powered-down.
  * @retval 0 if correct communication, else wrong communication
  */
void AUDIO_Stop(void)
{
//	wm8978_PowerDown();

 	{
		/* Stop the Transfer on the SAI side: Stop and disable the DMA stream */
		DMA_Cmd(PLAY_DMA_STREAM, DISABLE);
		
		/* Clear all the DMA flags for the next transfer */
		DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TC |PLAY_DMA_FLAG_HT | \
		                              PLAY_DMA_FLAG_FE | PLAY_DMA_FLAG_TE);
		
		/*  
		       The SAI DMA requests are not disabled here.
		                                                        */
		
		/* In all modes, disable the SAI peripheral */
		SAI_Cmd(SAI_BLOCK1, DISABLE);
	}
    
    /* Update the remaining data number */
    s_tPlay.AudioRemSize = s_tPlay.AudioTotalSize;    
    
}

/**
  * @brief  Controls the current audio volume level. 
  * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for 
  *         Mute and 100 for Max volume level).
  * @retval 0 if correct communication, else wrong communication
  */
void AUDIO_SetVolume(uint8_t Volume)
{
	/* ����������������ͬ���� */
	wm8978_SetEarVolume(Volume);
	wm8978_SetSpkVolume(Volume);
}


/*-----------------------------------------------------------------------------
                    Audio MAL Interface Control Functions
  ----------------------------------------------------------------------------*/

/**
  * @brief  Initializes and prepares the Media to perform audio data transfer 
  *         from Media to the SAI peripheral.
  * @param  None
  * @retval None
  */
static void Play_DMA_Init(void)  
{   
#if defined(PLAY_DMA_IT_TC_EN) || defined(PLAY_DMA_IT_HT_EN) || defined(PLAY_DMA_IT_TE_EN)
	NVIC_InitTypeDef NVIC_InitStructure;
#endif
	/* Enable the SAI  */
	SAI_Cmd(SAI_BLOCK1, DISABLE);
	
	/* Enable the DMA clock */
	RCC_AHB1PeriphClockCmd(PLAY_DMA_CLOCK, ENABLE);  // --- ��֪���Ƿ�������
	
	/* DISABLE the SAI DMA request */
	SAI_DMACmd(SAI_BLOCK1, DISABLE);
	
	/* Configure the DMA Stream */
	DMA_Cmd(PLAY_DMA_STREAM, DISABLE);
	DMA_DeInit(PLAY_DMA_STREAM);
	/* Set the parameters to be configured */
	DMA_Init_Play.DMA_Channel = PLAY_DMA_CHANNEL;  
	DMA_Init_Play.DMA_PeripheralBaseAddr = SAI_BLOCK1_DR;
	DMA_Init_Play.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
	DMA_Init_Play.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_Init_Play.DMA_BufferSize = (uint32_t)0xFFFF;      /* This field will be configured in play function */
	DMA_Init_Play.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Init_Play.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init_Play.DMA_PeripheralDataSize = PLAY_DMA_PERIPH_DATA_SIZE;
	DMA_Init_Play.DMA_MemoryDataSize = PLAY_DMA_MEM_DATA_SIZE; 
#ifdef AUDIO_MAL_MODE_NORMAL
	DMA_Init_Play.DMA_Mode = DMA_Mode_Normal;
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
	DMA_Init_Play.DMA_Mode = DMA_Mode_Circular;
#else
	#error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */  
	DMA_Init_Play.DMA_Priority = DMA_Priority_High;
	DMA_Init_Play.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_Init_Play.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_Init_Play.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_Init_Play.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	DMA_Init(PLAY_DMA_STREAM, &DMA_Init_Play);  
	
/* Enable the selected DMA interrupts (selected in "stm32_eval_audio_codec.h" defines) */
#ifdef PLAY_DMA_IT_TC_EN
	DMA_ITConfig(PLAY_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif /* PLAY_DMA_IT_TC_EN */

#ifdef PLAY_DMA_IT_HT_EN
	DMA_ITConfig(PLAY_DMA_STREAM, DMA_IT_HT, ENABLE);
#endif /* PLAY_DMA_IT_HT_EN */
	
#ifdef PLAY_DMA_IT_TE_EN
	DMA_ITConfig(PLAY_DMA_STREAM, DMA_IT_TE | DMA_IT_FE | DMA_IT_DME, ENABLE);
#endif /* PLAY_DMA_IT_TE_EN */
	
	/* Enable the SAI DMA request */
	SAI_DMACmd(SAI_BLOCK1, ENABLE);
	
#if defined(PLAY_DMA_IT_TC_EN) || defined(PLAY_DMA_IT_HT_EN) || defined(PLAY_DMA_IT_TE_EN)
	/* SAI DMA IRQ Channel configuration */
	NVIC_InitStructure.NVIC_IRQChannel = PLAY_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif 
}


/**
  * @brief  Initializes and prepares the Media to perform audio data transfer 
  *         from Media to the SAI peripheral.
  * @param  None
  * @retval None
  */
static void Rec_DMA_Init(void)  
{   
#if defined(REC_DMA_IT_TC_EN) || defined(REC_DMA_IT_HT_EN) || defined(REC_DMA_IT_TE_EN)
	NVIC_InitTypeDef NVIC_InitStructure;
#endif
	/* Enable the SAI  */
	SAI_Cmd(SAI_BLOCK2, DISABLE);
	
	/* Enable the DMA clock */
	RCC_AHB1PeriphClockCmd(REC_DMA_CLOCK, ENABLE); 
	
	/* Configure the DMA Stream */
	DMA_Cmd(REC_DMA_STREAM, DISABLE);
	DMA_DeInit(REC_DMA_STREAM);
	/* Set the parameters to be configured */
	DMA_Init_Rec.DMA_Channel = REC_DMA_CHANNEL;  
	DMA_Init_Rec.DMA_PeripheralBaseAddr = SAI_BLOCK2_DR;
	DMA_Init_Rec.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
	DMA_Init_Rec.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_Init_Rec.DMA_BufferSize = (uint32_t)0xFFFF;      /* This field will be configured in play function */
	DMA_Init_Rec.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Init_Rec.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init_Rec.DMA_PeripheralDataSize = REC_DMA_PERIPH_DATA_SIZE;
	DMA_Init_Rec.DMA_MemoryDataSize = REC_DMA_MEM_DATA_SIZE; 
#ifdef AUDIO_MAL_MODE_NORMAL
	DMA_Init_Rec.DMA_Mode = DMA_Mode_Normal;
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
	DMA_Init_Rec.DMA_Mode = DMA_Mode_Circular;
#else
	#error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */  
	DMA_Init_Rec.DMA_Priority = DMA_Priority_High;
	DMA_Init_Rec.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_Init_Rec.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_Init_Rec.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_Init_Rec.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	DMA_Init(REC_DMA_STREAM, &DMA_Init_Rec);  
	
/* Enable the selected DMA interrupts (selected in "stm32_eval_audio_codec.h" defines) */
#ifdef REC_DMA_IT_TC_EN
	DMA_ITConfig(REC_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif /* REC_DMA_IT_TC_EN */

#ifdef REC_DMA_IT_HT_EN
	DMA_ITConfig(REC_DMA_STREAM, DMA_IT_HT, ENABLE);
#endif /* REC_DMA_IT_HT_EN */
	
#ifdef REC_DMA_IT_TE_EN
	DMA_ITConfig(REC_DMA_STREAM, DMA_IT_TE | DMA_IT_FE | DMA_IT_DME, ENABLE);
#endif /* REC_DMA_IT_TE_EN */
	
	/* Enable the SAI DMA request */
	SAI_DMACmd(SAI_BLOCK2, ENABLE);
	
#if defined(REC_DMA_IT_TC_EN) || defined(REC_DMA_IT_HT_EN) || defined(REC_DMA_IT_TE_EN)
	/* SAI DMA IRQ Channel configuration */
	NVIC_InitStructure.NVIC_IRQChannel = REC_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif 
}

/**
  * @brief  Restore default state of the used Media.
  * @param  None
  * @retval None
  */
static void Play_SAI_DMA_DeInit(void)  
{   
#if defined(PLAY_DMA_IT_TC_EN) || defined(PLAY_DMA_IT_HT_EN) || defined(PLAY_DMA_IT_TE_EN)
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	/* Deinitialize the NVIC interrupt for the SAI DMA Stream */
	NVIC_InitStructure.NVIC_IRQChannel = PLAY_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);  
#endif 
	
	/* Disable the DMA stream before the deinit */
	DMA_Cmd(PLAY_DMA_STREAM, DISABLE);
	
	/* Dinitialize the DMA Stream */
	DMA_DeInit(PLAY_DMA_STREAM);
	
	/* 
	 The DMA clock is not disabled, since it can be used by other streams 
	 */   		                                                                      
}

/**
  * @brief  Restore default state of the used Media.
  * @param  None
  * @retval None
  */
static void Rec_SAI_DMA_DeInit(void)  
{   
#if defined(REC_DMA_IT_TC_EN) || defined(REC_DMA_IT_HT_EN) || defined(REC_DMA_IT_TE_EN)
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	/* Deinitialize the NVIC interrupt for the SAI DMA Stream */
	NVIC_InitStructure.NVIC_IRQChannel = PLAY_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);  
#endif 
	
	/* Disable the DMA stream before the deinit */
	DMA_Cmd(REC_DMA_STREAM, DISABLE);
	
	/* Dinitialize the DMA Stream */
	DMA_DeInit(REC_DMA_STREAM);
	
	/* 
	 The DMA clock is not disabled, since it can be used by other streams 
	*/ 
 	  		                                                                                                                                   
}

/**
  * @brief  This function handles main Media layer interrupt. 
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
void PLAY_DMA_IRQHandler(void)
{    
	#ifdef PLAY_DMA_IT_TC_EN
	/* Transfer complete interrupt --- */
		if (DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TC) != RESET)
		{     
		#ifdef AUDIO_MAL_MODE_NORMAL
			/* Check if the end of file has been reached */
			if (s_tPlay.AudioRemSize > DMA_MAX_SZE)
			{      
				/* Wait the DMA Stream to be effectively disabled */
				while (DMA_GetCmdStatus(PLAY_DMA_STREAM) != DISABLE)
				{}
				
				/* Clear the Interrupt flag */
				DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_ALL);  
				
				/* Re-Configure the buffer address and size */
				DMA_Init_Play.DMA_Memory0BaseAddr = (uint32_t) s_tPlay.CurrentPos;
				DMA_Init_Play.DMA_BufferSize = (uint32_t) (DMA_MAX(s_tPlay.AudioRemSize));
				
				/* Configure the DMA Stream with the new parameters */
				DMA_Init(PLAY_DMA_STREAM, &DMA_Init_Play);
				
				/* Enable the SAI DMA Stream*/
				DMA_Cmd(PLAY_DMA_STREAM, ENABLE);    
				
				/* Update the current pointer position */
				s_tPlay.CurrentPos += DMA_MAX(s_tPlay.AudioRemSize);        
				
				/* Update the remaining number of data to be played */
				s_tPlay.AudioRemSize -= DMA_MAX(s_tPlay.AudioRemSize);    
			}
			else
			{
				/* Disable the SAI DMA Stream*/
				DMA_Cmd(PLAY_DMA_STREAM, DISABLE);   
				
				/* Clear the Interrupt flag */
				DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TC);       
				
				/* �˴����� bsp_PutMsg() һ����Ϣ��֪ͨ������DMA������� */
				//should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h)
				//EVAL_AUDIO_TransferComplete_CallBack((uint32_t)CurrentPos, 0);      
				/* ����һ����Ϣ��DMA���� */
				bsp_PutMsg(MSG_WM8978_DMA_END, 0);			
			}
		#elif defined(AUDIO_MAL_MODE_CIRCULAR)
			/* Manage the remaining file size and new address offset: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			//EVAL_AUDIO_TransferComplete_CallBack((uint32_t)CurrentPos, AudioRemSize); 
			/* ����һ����Ϣ��DMA���� */
			bsp_PutMsg(MSG_WM8978_DMA_END, 0);			
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TC);
		#endif /* AUDIO_MAL_MODE_NORMAL */  
		}
	#endif /* PLAY_DMA_IT_TC_EN */
	
	#ifdef PLAY_DMA_IT_HT_EN  
		/* Half Transfer complete interrupt */
		if (DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_HT) != RESET)
		{
			/* Manage the remaining file size and new address offset: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			//EVAL_AUDIO_HalfTransfer_CallBack((uint32_t)CurrentPos, AudioRemSize);    
			
			/* ����һ����Ϣ��DMA���� */
			bsp_PutMsg(MSG_WM8978_DMA_END, 0);
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_HT);    
		}
	#endif /* PLAY_DMA_IT_HT_EN */
	
	#ifdef PLAY_DMA_IT_TE_EN  
		/* FIFO Error interrupt */
		if ((DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TE) != RESET) || \
		(DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_FE) != RESET) || \
		(DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_DME) != RESET))
		{
			/* Manage the error generated on DMA FIFO: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			//EVAL_AUDIO_Error_CallBack((uint32_t*)&CurrentPos);    
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TE | PLAY_DMA_FLAG_FE | \
			                        PLAY_DMA_FLAG_DME);
		}  
	#endif /* PLAY_DMA_IT_TE_EN */
}

/* ¼����DMAͨ���жϷ������ */
void REC_DMA_IRQHandler(void)
{
	#ifdef REC_DMA_IT_TC_EN
	/* Transfer complete interrupt --- */
		if (DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_TC) != RESET)
		{     
		#ifdef AUDIO_MAL_MODE_NORMAL
			/* Check if the end of file has been reached */
			if (s_tRec.AudioRemSize > DMA_MAX_SZE)
			{      
				/* Wait the DMA Stream to be effectively disabled */
				while (DMA_GetCmdStatus(REC_DMA_STREAM) != DISABLE)
				{}
				
				/* Clear the Interrupt flag */
				DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_ALL);  
				
				/* Re-Configure the buffer address and size */
				DMA_Init_Rec.DMA_Memory0BaseAddr = (uint32_t) s_tRec.CurrentPos;
				DMA_Init_Rec.DMA_BufferSize = (uint32_t) (DMA_MAX(s_tRec.AudioRemSize));
				
				/* Configure the DMA Stream with the new parameters */
				DMA_Init(REC_DMA_STREAM, &DMA_Init_Rec);
				
				/* Enable the SAI DMA Stream*/
				DMA_Cmd(REC_DMA_STREAM, ENABLE);    
				
				/* Update the current pointer position */
				s_tRec.CurrentPos += DMA_MAX(s_tRec.AudioRemSize);        
				
				/* Update the remaining number of data to be record */
				s_tRec.AudioRemSize -= DMA_MAX(s_tRec.AudioRemSize);    
			}
			else
			{
				/* Disable the SAI DMA Stream*/
				DMA_Cmd(REC_DMA_STREAM, DISABLE);   
				
				/* Clear the Interrupt flag */
				DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_TC);       
				
				/* Manage the remaining file size and new address offset: This function 
				should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
				/* ����һ����Ϣ��DMA���� */
				bsp_PutMsg(MSG_WM8978_DMA_END, 0);
			}
		#elif defined(AUDIO_MAL_MODE_CIRCULAR)
			/* Manage the remaining file size and new address offset: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			//EVAL_AUDIO_TransferComplete_CallBack((uint32_t)s_tRec.CurrentPos, s_tRec.AudioRemSize);    
			/* ����һ����Ϣ��DMA���� */
			bsp_PutMsg(MSG_WM8978_DMA_END, 0);
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_TC);
		#endif /* AUDIO_MAL_MODE_NORMAL */  
		}
	#endif /* REC_DMA_IT_TC_EN */
	
	#ifdef REC_DMA_IT_HT_EN  
		/* Half Transfer complete interrupt */
		if (DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_HT) != RESET)
		{
			/* Manage the remaining file size and new address offset: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			EVAL_AUDIO_HalfTransfer_CallBack((uint32_t)s_tRec.CurrentPos, s_tRec.AudioRemSize);    
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_HT);    
		}
	#endif /* REC_DMA_IT_HT_EN */
	
	#ifdef REC_DMA_IT_TE_EN  
		/* FIFO Error interrupt */
		if ((DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_TE) != RESET) || \
		(DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_FE) != RESET) || \
		(DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_DME) != RESET))
		{
			/* Manage the error generated on DMA FIFO: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			EVAL_AUDIO_Error_CallBack((uint32_t*)&s_tRec.CurrentPos);    
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_TE | REC_DMA_FLAG_FE | \
			                        REC_DMA_FLAG_DME);
		}  
	#endif /* REC_DMA_IT_TE_EN */
}


/*
*********************************************************************************************************
*	�� �� ��: SAI_GPIO_Config
*	����˵��: ����GPIO��������SAI codecӦ��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SAI_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*
		������STM32-V6������---  SAI�ӿ� I2S���ߴ�����Ƶ���ݿ���	
			PF9/SAI1_FS_B
			PF8/SAI1_SCK_B
			PD6/SAI1_SD_A
			PF6/SAI1_SD_B
			PF7/SAI1_MCLK_B	
	*/
	
	/* ʹ��SAI��Ƶ�ӿڵ�GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(SAI_GPIO_RCC, ENABLE);
	
	/* ���� FS, SCK, MCK, SD1�� SD2 */
	GPIO_PinAFConfig(SAI_GPIO_FS, SAI_PINSRC_FS, SAI_GPIO_AF); 
	GPIO_PinAFConfig(SAI_GPIO_SCK, SAI_PINSRC_SCK, SAI_GPIO_AF);  
	GPIO_PinAFConfig(SAI_GPIO_MCK, SAI_PINSRC_MCK, SAI_GPIO_AF); 
	GPIO_PinAFConfig(SAI_GPIO_SD1, SAI_PINSRC_SD1, SAI_GPIO_AF); 
	GPIO_PinAFConfig(SAI_GPIO_SD2, SAI_PINSRC_SD2, SAI_GPIO_AF);

	/* Configure pins as AF pushpull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = SAI_PIN_MCK;	
	GPIO_Init(SAI_GPIO_MCK, &GPIO_InitStructure); 
		
	
	/* Configure pins as AF pushpull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	
	GPIO_InitStructure.GPIO_Pin = SAI_PIN_FS;	
	GPIO_Init(SAI_GPIO_FS, &GPIO_InitStructure); 	

	GPIO_InitStructure.GPIO_Pin = SAI_PIN_SCK;	
	GPIO_Init(SAI_GPIO_SCK, &GPIO_InitStructure); 
	

	GPIO_InitStructure.GPIO_Pin = SAI_PIN_SD1;	
	GPIO_Init(SAI_GPIO_SD1, &GPIO_InitStructure); 	

	/* Configure pins as AF pushpull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = SAI_PIN_SD2;	
	GPIO_Init(SAI_GPIO_SD2, &GPIO_InitStructure); 	
}

/*
*********************************************************************************************************
*	�� �� ��: SAI_Mode_Config
*	����˵��: ����STM32��SAI��Ƶ�ӿڵĹ���ģʽ�� ����
*	��    ��:  _mode : 1��ʾ¼��, 2��ʾ����, 3��ʾ¼������ͬʱ������δʵ�֣�
*			  _usStandard :  �ӿڱ�׼ I2S_Standard_Phillips,  I2S_Standard_MSB  ��  I2S_Standard_LSB
*			  _uiWordLen : �����ֳ�����ѡ�Ĳ���: SAI_DataSize_8b,SAI_DataSize_10b 
*						SAI_DataSize_16b, SAI_DataSize_20b, SAI_DataSize_24b, SAI_DataSize_32b
*					Ŀǰ������֧��16bit��
*				WM8978֧��    16��20��24��32bit��
*				STM32F492֧�� 8��10��16��20��24��31bit
*							
*			  _usAudioFreq : ����Ƶ�ʣ�I2S_AudioFreq_8K��I2S_AudioFreq_16K��I2S_AudioFreq_22K��
*							I2S_AudioFreq_44K��I2S_AudioFreq_48
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void SAI_Mode_Config(uint8_t _mode, uint16_t _usStandard, uint32_t _uiWordLen, uint32_t _uiAudioFreq)
{
	uint32_t tmpdiv;
	uint32_t SAI_ClockSrcFreq;

	SAI_InitTypeDef       SAI_InitStructure;
	SAI_FrameInitTypeDef  SAI_FrameInitStructure;
	SAI_SlotInitTypeDef   SAI_SlotInitStructure;

	/* 
		����MCKʱ�������Ƶ�� 
		
		MCLK_x = SAI_CK_x / (MCKDIV[3:0] * 2) with MCLK_x = 256 * FS
		
		FS = SAI_CK_x / (MCKDIV[3:0] * 2) * 256
		MCKDIV[3:0] = SAI_CK_x / FS * 512 	
	
		���ݲ���Ƶ��ѡ��ʱ��Դ, ������Ƶ����Եõ�׼ȷ��λʱ�� 
		#define SAI_ClockPLLSAI             ((uint32_t)11289600)
		#define SAI_ClockPLLI2S             ((uint32_t)49152000)

	*/
	if (_uiAudioFreq == SAI_AudioFreq_44_1k || _uiAudioFreq == SAI_AudioFreq_22_05k
		|| _uiAudioFreq == SAI_AudioFreq_11_025k)
	{
		SAI_ClockSrcFreq = SAI_ClockPLLSAI;
		
		/* ����SAI_Block_A�� SAI_Block_B��ʱ��Դ */
		RCC_SAIBlockACLKConfig(RCC_SAIACLKSource_PLLSAI);
		RCC_SAIBlockBCLKConfig(RCC_SAIACLKSource_PLLSAI);		
	}
	else	/*192k, 96k, 48k, 32k, 16k, 8k */
	{
		if (_uiAudioFreq ==	SAI_AudioFreq_8k)
		{
			RCC_PLLI2SConfig(344, 14, 4);	// 49152000 / 2;
			SAI_ClockSrcFreq = 49152000 / 2;
		}
		else
		{
			RCC_PLLI2SConfig(344, 7, 4);	// 49152000
			SAI_ClockSrcFreq = 49152000;
		}	

		/* ����SAI_Block_A�� SAI_Block_B��ʱ��Դ */
		RCC_SAIBlockACLKConfig(RCC_SAIBCLKSource_PLLI2S);
		RCC_SAIBlockBCLKConfig(RCC_SAIBCLKSource_PLLI2S);		
	}
	
	tmpdiv = SAI_ClockSrcFreq / (_uiAudioFreq * 256);
  
	SAI_InitStructure.SAI_NoDivider = SAI_MasterDivider_Enabled;
	SAI_InitStructure.SAI_MasterDivider = tmpdiv;

	/* 
		[�ڲ�ͬ��]
		1. ��Ƶģ�������Ϊ��ڶ�����Ƶģ��ͬ��������������£�������λʱ�Ӻ�֡ͬ���źţ���
		   ����ͨ��ʱռ���ⲿ���ŵ�������
		2. ����Ϊ����һ��ģ��ͬ������Ƶģ�齫�ͷ��� SCK_x��FS_x �� MCLK_x ���������� GPIO��
		3. ����Ϊ�첽��ģ�齫ʹ���� I/O ���� FS_x�� SCK_x �� MCLK_x���������Ƶģ�鱻��Ϊ��ģ�飩��
			
		ͨ������Ƶģ��ͬ��ģʽ��������ȫ˫��ģʽ������ SAI��������Ƶģ�������һ��������Ϊ��ģ�飬��һ��Ϊ��ģ�飻
		Ҳ�ɽ�����������Ϊ��ģ�飻һ��ģ������Ϊ�첽 SAI_xCR1 �е���Ӧλ SYNCEN[1:0] = 00����
		  ��һ������Ϊͬ���� SAI_xCR1 �е���ӦλSYNCEN[1:0] = 01����
		  
		ע�⣺ ���ڴ����ڲ�����ͬ���׶Σ� APB Ƶ�� PCLK ������ڻ���ڱ�����ʱ��Ƶ�ʵĶ�����
		
		������ʹ����ģʽǰʹ�ܴ�ģʽ��
	*/	
	if (_mode == 1)	/* ���� */
	{
		/* ������Ƶ��ģ��1 --- ������ �� ������ģ��ͬ��ģʽ���� */
		SAI_InitStructure.SAI_AudioMode = SAI_Mode_MasterTx;	/* ����Ϊ��ģʽ���� */
		SAI_InitStructure.SAI_Protocol = SAI_Free_Protocol;		/* Э�� */
		SAI_InitStructure.SAI_DataSize = _uiWordLen;			/* �����ֳ� */
		SAI_InitStructure.SAI_FirstBit = SAI_FirstBit_MSB;		/* bit���򣬸�bit�ȴ� */
		SAI_InitStructure.SAI_ClockStrobing = SAI_ClockStrobing_RisingEdge;
		SAI_InitStructure.SAI_Synchro = SAI_Asynchronous;		/* ����Ϊ�첽��ʹ�ñ�ģ��� FS, SCK,MCLK */
		SAI_InitStructure.SAI_OUTDRIV = SAI_OutputDrive_Enabled;
		SAI_InitStructure.SAI_FIFOThreshold = SAI_FIFOThreshold_1QuarterFull;
		SAI_Init(SAI_BLOCK1, &SAI_InitStructure);
	
	
		/* Configure SAI_Block_x Frame 
			Frame Length : 32
			Frame active Length: 16
			FS Definition : Start frame + Channel Side identification
			FS Polarity: FS active Low
			FS Offset: FS asserted one bit before the first bit of slot 0 */ 
		/*
			��֡����ӦΪ 8 �� 256 ֮���һ������ 2
			�� n ���ݵ���������Ϊ��ȷ����Ƶ֡��ÿ��λʱ�Ӱ��������� MCLK ���壬������ȷ���������ڵ��ⲿ DAC/ADC ��ȷ������	
			
			
		*/
		SAI_FrameInitStructure.SAI_FrameLength = 32;		/* ST������64�� V6��32 */
		SAI_FrameInitStructure.SAI_ActiveFrameLength = 16;  
		SAI_FrameInitStructure.SAI_FSDefinition = I2S_FS_ChannelIdentification;	 /* FS����Ϊ�������� */
		SAI_FrameInitStructure.SAI_FSPolarity = SAI_FS_ActiveLow;  
		SAI_FrameInitStructure.SAI_FSOffset = SAI_FS_BeforeFirstBit;
		SAI_FrameInit(SAI_BLOCK1, &SAI_FrameInitStructure);
	
		/* ���� SAI Block_x Slot 
		Slot First Bit Offset : 0
		Slot Size   : 16
		Slot Number : 2     <---- ST�ٷ������õ�4��Slot����������������2������������ ������V6����WM8978��2��Slot��ʾ����������
		Slot Active : All slot actives 
		*/
		SAI_SlotInitStructure.SAI_FirstBitOffset = 0;
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_16b; 
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_DataSize;
		SAI_SlotInitStructure.SAI_SlotNumber = 2;		/* 2������, ÿ������һ�� Slot */
		SAI_SlotInitStructure.SAI_SlotActive =  SAI_SlotActive_0;
		SAI_SlotInit(SAI_BLOCK1, &SAI_SlotInitStructure); 
	
		SAI_FlushFIFO(SAI_BLOCK1);
	}
	else if (_mode == 2)	/* ¼�� */
	{
		/* ������Ƶ��ģ��1 --- ������ �� ������ģ��ͬ��ģʽ���� */
		SAI_InitStructure.SAI_AudioMode = SAI_Mode_MasterRx;	/* ����Ϊ��ģʽ���� */
		SAI_InitStructure.SAI_Protocol = SAI_Free_Protocol;		/* Э�� */
		SAI_InitStructure.SAI_DataSize = _uiWordLen;			/* �����ֳ� */
		SAI_InitStructure.SAI_FirstBit = SAI_FirstBit_MSB;		/* bit���򣬸�bit�ȴ� */
		SAI_InitStructure.SAI_ClockStrobing = SAI_ClockStrobing_RisingEdge;
		SAI_InitStructure.SAI_Synchro = SAI_Asynchronous;		/* ����Ϊ�첽��ʹ�ñ�ģ��� FS, SCK,MCLK */
		SAI_InitStructure.SAI_OUTDRIV = SAI_OutputDrive_Enabled;
		SAI_InitStructure.SAI_FIFOThreshold = SAI_FIFOThreshold_1QuarterFull;
		SAI_Init(SAI_BLOCK2, &SAI_InitStructure);
	
	
		/* Configure SAI_Block_x Frame 
			Frame Length : 32
			Frame active Length: 16
			FS Definition : Start frame + Channel Side identification
			FS Polarity: FS active Low
			FS Offset: FS asserted one bit before the first bit of slot 0 */ 
		/*
			��֡����ӦΪ 8 �� 256 ֮���һ������ 2
			�� n ���ݵ���������Ϊ��ȷ����Ƶ֡��ÿ��λʱ�Ӱ��������� MCLK ���壬������ȷ���������ڵ��ⲿ DAC/ADC ��ȷ������	
			
			
		*/
		SAI_FrameInitStructure.SAI_FrameLength = 32;		/* ST������64�� V6��32 */
		SAI_FrameInitStructure.SAI_ActiveFrameLength = 16;  
		SAI_FrameInitStructure.SAI_FSDefinition = I2S_FS_ChannelIdentification;	 /* FS����Ϊ�������� */
		SAI_FrameInitStructure.SAI_FSPolarity = SAI_FS_ActiveLow;  
		SAI_FrameInitStructure.SAI_FSOffset = SAI_FS_BeforeFirstBit;
		SAI_FrameInit(SAI_BLOCK2, &SAI_FrameInitStructure);
	
		/* ���� SAI Block_x Slot 
		Slot First Bit Offset : 0
		Slot Size   : 16
		Slot Number : 2     <---- ST�ٷ������õ�4��Slot����������������2������������ ������V6����WM8978��2��Slot��ʾ����������
		Slot Active : All slot actives 
		*/
		SAI_SlotInitStructure.SAI_FirstBitOffset = 0;
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_16b; 
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_DataSize;
		SAI_SlotInitStructure.SAI_SlotNumber = 2;		/* 2������, ÿ������һ�� Slot */
		SAI_SlotInitStructure.SAI_SlotActive =  SAI_SlotActive_0;
		SAI_SlotInit(SAI_BLOCK2, &SAI_SlotInitStructure); 
	
		SAI_FlushFIFO(SAI_BLOCK2);		
	}
	else if (_mode == 3)	/* ¼���ͷ���ͬʱ */
	{
		/* ������Ƶ��ģ��2 --- ¼���� */
		SAI_InitStructure.SAI_AudioMode = SAI_Mode_SlaveRx;		/* ����Ϊ��ģʽ���� */
		SAI_InitStructure.SAI_Protocol = SAI_Free_Protocol;		/* Э�� */
		SAI_InitStructure.SAI_DataSize = _uiWordLen;		/* �ֳ�������ע�� SAI_DataSize_16b ���� 16 */
		SAI_InitStructure.SAI_FirstBit = SAI_FirstBit_MSB;		/* ��׼I2S��ʽ�����Ǹ�bit�ȴ��� */
		SAI_InitStructure.SAI_ClockStrobing = SAI_ClockStrobing_RisingEdge;
		SAI_InitStructure.SAI_Synchro = SAI_Synchronous;		/* ѡͬ��ģʽ, */
		SAI_InitStructure.SAI_OUTDRIV = SAI_OutputDrive_Disabled;
		SAI_InitStructure.SAI_FIFOThreshold = SAI_FIFOThreshold_1QuarterFull;
		SAI_Init(SAI_BLOCK2, &SAI_InitStructure);		
	
		/* ������Ƶ��ģ��1 --- ������ �� ������ģ��ͬ��ģʽ���� */
		SAI_InitStructure.SAI_AudioMode = SAI_Mode_MasterTx;	/* ����Ϊ��ģʽ���� */
		SAI_InitStructure.SAI_Protocol = SAI_Free_Protocol;		/* Э�� */
		SAI_InitStructure.SAI_DataSize = _uiWordLen;			/* �����ֳ� */
		SAI_InitStructure.SAI_FirstBit = SAI_FirstBit_MSB;		/* bit���򣬸�bit�ȴ� */
		SAI_InitStructure.SAI_ClockStrobing = SAI_ClockStrobing_RisingEdge;
		SAI_InitStructure.SAI_Synchro = SAI_Asynchronous;		/* ����Ϊ�첽��ʹ�ñ�ģ��� FS, SCK,MCLK */
		SAI_InitStructure.SAI_OUTDRIV = SAI_OutputDrive_Enabled;
		SAI_InitStructure.SAI_FIFOThreshold = SAI_FIFOThreshold_1QuarterFull;
		SAI_Init(SAI_BLOCK1, &SAI_InitStructure);
	
	
		/* Configure SAI_Block_x Frame 
			Frame Length : 32
			Frame active Length: 16
			FS Definition : Start frame + Channel Side identification
			FS Polarity: FS active Low
			FS Offset: FS asserted one bit before the first bit of slot 0 */ 
		/*
			��֡����ӦΪ 8 �� 256 ֮���һ������ 2
			�� n ���ݵ���������Ϊ��ȷ����Ƶ֡��ÿ��λʱ�Ӱ��������� MCLK ���壬������ȷ���������ڵ��ⲿ DAC/ADC ��ȷ������	
			
			
		*/
		SAI_FrameInitStructure.SAI_FrameLength = 32;		/* ST������64�� V6��32 */
		SAI_FrameInitStructure.SAI_ActiveFrameLength = 16;  
		SAI_FrameInitStructure.SAI_FSDefinition = I2S_FS_ChannelIdentification;	 /* FS����Ϊ�������� */
		SAI_FrameInitStructure.SAI_FSPolarity = SAI_FS_ActiveLow;  
		SAI_FrameInitStructure.SAI_FSOffset = SAI_FS_BeforeFirstBit;
		SAI_FrameInit(SAI_BLOCK1, &SAI_FrameInitStructure);
		
		SAI_FrameInit(SAI_BLOCK2, &SAI_FrameInitStructure);
	
		/* ���� SAI Block_x Slot 
		Slot First Bit Offset : 0
		Slot Size   : 16
		Slot Number : 2     <---- ST�ٷ������õ�4��Slot����������������2������������ ������V6����WM8978��2��Slot��ʾ����������
		Slot Active : All slot actives 
		*/
		SAI_SlotInitStructure.SAI_FirstBitOffset = 0;
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_16b; 
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_DataSize;
		SAI_SlotInitStructure.SAI_SlotNumber = 2;		/* 2������, ÿ������һ�� Slot */
		SAI_SlotInitStructure.SAI_SlotActive =  SAI_SlotActive_0;
		SAI_SlotInit(SAI_BLOCK1, &SAI_SlotInitStructure); 
		
		SAI_SlotInit(SAI_BLOCK2, &SAI_SlotInitStructure); 
	
		SAI_FlushFIFO(SAI_BLOCK1);
		SAI_FlushFIFO(SAI_BLOCK2);
	}
}

/**
  * @brief  Configure PLLSAI. ��������ʱ��Դ��Ӧ�Գ��õ���Ƶ����Ƶ��.
  * @param  None
  * @retval None
  */

/*
*********************************************************************************************************
*	�� �� ��: PLLSAI_Config
*	����˵��: ����SAIʱ��Դ��ѡ������ʱ��Դ������Գ�������Ƶ����Ƶ��.
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void PLLSAI_Config(void)
{ 
	/* Configure PLLSAI prescalers */
	/* PLLSAI_VCO : VCO_429M */
	/* SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz */
	RCC_PLLSAIConfig(429, 2, 4);

	/* SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz */  
	RCC_SAIPLLSAIClkDivConfig(19);

	/* Configure PLLI2S prescalers */
	/* PLLI2S_VCO : VCO_344M */
	/* SAI_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 344/7 = 49.142 Mhz */
	RCC_PLLI2SConfig(344, 7, 4);

	/* SAI_CLK_x = SAI_CLK(first level)/PLLI2SDIVQ = 49.142/1 = 49.142 Mhz */  
	RCC_SAIPLLI2SClkDivConfig(1);

	/* Configure Clock source for SAI Block A */
	RCC_SAIBlockACLKConfig(RCC_SAIACLKSource_PLLSAI);

	/* Configure Clock source for SAI Block B */
	RCC_SAIBlockBCLKConfig(RCC_SAIBCLKSource_PLLI2S);

	/* Enable PLLSAI Clock */
	RCC_PLLSAICmd(ENABLE);

	/* Wait till PLLSAI is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET) 
	{
	}

	/* Enable PLLI2S Clock */
	RCC_PLLI2SCmd(ENABLE);

	/* Wait till PLLI2S is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY) == RESET) 
	{
	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
