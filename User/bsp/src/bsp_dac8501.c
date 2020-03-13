/*
*********************************************************************************************************
*
*	ģ������ : DAC8501 ����ģ��(��ͨ����16λDAC)
*	�ļ����� : bsp_dac8501.c
*	��    �� : V1.0
*	˵    �� : DAC8501ģ���CPU֮�����SPI�ӿڡ�����������֧��Ӳ��SPI�ӿں����SPI�ӿڡ�
*			  ͨ�����л���
*
*	�޸ļ�¼ :
*		�汾��  ����         ����     ˵��
*		V1.0    2015-10-11  armfly  ��ʽ����
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	DAC8501ģ�����ֱ�Ӳ嵽STM32-V6������CN19��ĸ(2*4P 2.54mm)�ӿ���

    DAC8501ģ��    STM32-V6������
	  VCC   ------  3.3V
	  GND   ------  GND
      SCLK  ------  PB3/SPI3_SCK
      MOSI  ------  PB5/SPI3_MOSI
      CS1   ------  PG10/NRF24L01_CSN
	  CS2   ------  ��չIO - NRF24L01_CE
			------  PB4/SPI3_MISO
			------  PE4/NRF24L01_IRQ

*/

/*
	DAC8501��������:
	1������2.7 - 5V;  ������ʹ��3.3V��
	4���ο���ѹ2.5V (�Ƽ�ȱʡ�ģ����õģ�

	��SPI��ʱ���ٶ�Ҫ��: �ߴ�30MHz�� �ٶȺܿ�.
	SCLK�½��ض�ȡ����, ÿ�δ���24bit���ݣ� ��λ�ȴ�
*/

/* ������У���ʾʹ����չ��IO */
#define USE_HC574

#define DAC8501_RCC_CS1 	RCC_AHB1Periph_GPIOG
#define DAC8501_PORT_CS1	GPIOG
#define DAC8501_PIN_CS1		GPIO_Pin_10
#define DAC8501_CS1_1()		DAC8501_PORT_CS1->BSRRL = DAC8501_PIN_CS1
#define DAC8501_CS1_0()		DAC8501_PORT_CS1->BSRRH = DAC8501_PIN_CS1

#ifdef USE_HC574	/* ʹ����չIO */
	#define DAC8501_CS2_1()		HC574_SetPin(NRF24L01_CE, 1);
	#define DAC8501_CS2_0()		HC574_SetPin(NRF24L01_CE, 0);
#else
	#define DAC8501_RCC_CS2 	RCC_AHB1Periph_GPIOA
	#define DAC8501_PORT_CS2	GPIOA
	#define DAC8501_PIN_CS2		GPIO_Pin_4
	#define DAC8501_CS2_1()		DAC8501_PORT_CS2->BSRRL = DAC8501_PIN_CS2
	#define DAC8501_CS2_0()		DAC8501_PORT_CS2->BSRRH = DAC8501_PIN_CS2
#endif

/* �����ѹ��DACֵ��Ĺ�ϵ�� ����У׼ x��dac y �ǵ�ѹ 0.1mV */
#define X1	100
#define Y1  50

#define X2	65000
#define Y2  49400

static void DAC8501_ConfigGPIO(void);

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitDAC8501
*	����˵��: ����STM32��GPIO��SPI�ӿڣ��������� ADS1256
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitDAC8501(void)
{
	DAC8501_ConfigGPIO();

	DAC8501_SetDacData(0, 0);
	DAC8501_SetDacData(1, 0);
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_CfgSpiHard
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ���ٶȵȲ��������ڷ���TM7705
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8501_CfgSpiHard(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* ���� SPI1����ģʽ */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 		/* �������Ƭѡ */

	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	/* ʹ�� SPI1 */
	SPI_Cmd(SPI1,ENABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_SetCS1(0)
*	����˵��: ����CS1�� ����������SPI����
*	��    ��: ��
	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8501_SetCS1(uint8_t _level)
{
	if (_level == 0)
	{
		bsp_SpiBusEnter();	/* ռ��SPI���ߣ� �������߹��� */

		#ifdef SOFT_SPI		/* ���SPI */
			bsp_SetSpiSck(0);
			DAC8501_CS1_0();
		#endif

		#ifdef HARD_SPI		/* Ӳ��SPI */
			bsp_SPI_Init(SPI_Direction_2Lines_FullDuplex | SPI_Mode_Master | SPI_DataSize_8b
				| SPI_CPOL_Low | SPI_CPHA_1Edge | SPI_NSS_Soft | SPI_BaudRatePrescaler_8 | SPI_FirstBit_MSB);

			DAC8501_CS1_0();
		#endif
	}
	else
	{
		DAC8501_CS1_1();

		bsp_SpiBusExit();	/* �ͷ�SPI���ߣ� �������߹��� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_SetCS2(0)
*	����˵��: ����CS2�� ����������SPI����
*	��    ��: ��
	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8501_SetCS2(uint8_t _level)
{
	if (_level == 0)
	{
		bsp_SpiBusEnter();	/* ռ��SPI���ߣ� �������߹��� */

		#ifdef SOFT_SPI		/* ���SPI */
			bsp_SetSpiSck(0);
			DAC8501_CS2_0();
		#endif

		#ifdef HARD_SPI		/* Ӳ��SPI */
			bsp_SPI_Init(SPI_Direction_2Lines_FullDuplex | SPI_Mode_Master | SPI_DataSize_8b
				| SPI_CPOL_Low | SPI_CPHA_1Edge | SPI_NSS_Soft | SPI_BaudRatePrescaler_8 | SPI_FirstBit_MSB);

			DAC8501_CS2_0();
		#endif
	}
	else
	{
		DAC8501_CS2_1();

		bsp_SpiBusExit();	/* �ͷ�SPI���ߣ� �������߹��� */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_ConfigGPIO
*	����˵��: ����GPIO�� ������ SCK  MOSI  MISO �����SPI���ߡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DAC8501_ConfigGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ��GPIOʱ�� */
	RCC_AHB1PeriphClockCmd(DAC8501_RCC_CS1, ENABLE);

	/* ���ü����������IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = DAC8501_PIN_CS1;
	GPIO_Init(DAC8501_PORT_CS1, &GPIO_InitStructure);

#ifdef USE_HC574	/* CS2 ʹ����չIO */
	;
#else
	RCC_AHB1PeriphClockCmd(DAC8501_RCC_CS1, ENABLE);

	/* ���ü����������IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* ��Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO������ٶ� */	
	
	GPIO_InitStructure.GPIO_Pin = DAC8501_PIN_CS2;
	GPIO_Init(DAC8501_PORT_CS2, &GPIO_InitStructure);
#endif

}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_SetDacData
*	����˵��: ����DAC����
*	��    ��: _ch, ͨ��,
*		     _data : ����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DAC8501_SetDacData(uint8_t _ch, uint16_t _dac)
{
	uint32_t data;

	/*
		DAC8501.pdf page 12 ��24bit����

		DB24:18 = xxxxx ����
		DB17�� PD1
		DB16�� PD0

		DB15��0  16λ����

		���� PD1 PD0 ����4�ֹ���ģʽ
		      0   0  ---> ��������ģʽ
		      0   1  ---> �����1Kŷ��GND
		      1   0  ---> ���100Kŷ��GND
		      1   1  ---> �������
	*/

	data = _dac; /* PD1 PD0 = 00 ����ģʽ */

	if (_ch == 0)
	{
		DAC8501_SetCS1(0);
	}
	else
	{
		DAC8501_SetCS2(0);
	}

	/*��DAC8501 SCLKʱ�Ӹߴ�30M����˿��Բ��ӳ� */
	bsp_spiWrite0(data >> 16);
	bsp_spiWrite0(data >> 8);
	bsp_spiWrite0(data);

	if (_ch == 0)
	{
		DAC8501_SetCS1(1);
	}
	else
	{
		DAC8501_SetCS2(1);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_DacToVoltage
*	����˵��: ��DACֵ����Ϊ��ѹֵ����λ0.1mV
*	��    ��: _dac  16λDAC��
*	�� �� ֵ: ��ѹ����λ0.1mV
*********************************************************************************************************
*/
int32_t DAC8501_DacToVoltage(uint16_t _dac)
{
	int32_t y;

	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	y =  CaculTwoPoint(X1, Y1, X2, Y2, _dac);
	if (y < 0)
	{
		y = 0;
	}
	return y;
}

/*
*********************************************************************************************************
*	�� �� ��: DAC8501_DacToVoltage
*	����˵��: ��DACֵ����Ϊ��ѹֵ����λ 0.1mV
*	��    ��: _volt ��ѹ����λ0.1mV
*	�� �� ֵ: 16λDAC��
*********************************************************************************************************
*/
uint32_t DAC8501_VoltageToDac(int32_t _volt)
{
	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	return CaculTwoPoint(Y1, X1, Y2, X2, _volt);
}



/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
