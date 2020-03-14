/*
*********************************************************************************************************
*
*	模块名称 : DAC8562/8563 驱动模块(单通道带16位DAC)
*	文件名称 : bsp_dac8562.c
*	版    本 : V1.1
*	说    明 : DAC8562/8563模块和CPU之间采用SPI接口。本驱动程序支持硬件SPI接口和软件SPI接口。
*			  通过宏切换。
*
*	修改记录 :
*		版本号  日期         作者     说明
*		V1.0    2014-01-17  armfly  正式发布
*		V1.1    2015-10-09  armfly  修正接线错误，LDAC引脚需要接地，CLR也需要接地
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	DAC8562模块可以直接插到STM32-V6开发板CN19排母(2*4P 2.54mm)接口上

    DAC8562/8563模块    STM32-V6开发板
	  GND   ------  GND
	  VCC   ------  3.3V

	  LDAC  ------  扩展IO/NRF24L01_CE/DAC1_OUT    --- 必须接地或发指令前设0V
      SYNC  ------  PG10/NRF24L01_CSN

      SCLK  ------  PB3/SPI3_SCK
      DIN   ------  PB5/SPI3_MOSI

			------  PB4/SPI3_MISO         ---- DAC无读出需求
	  CLR   ------  PE4/NRF24L01_IRQ	  ---  推荐接GND。
*/

/*
	DAC8562基本特性:
	1、供电2.7 - 5V;  【本例使用3.3V】
	4、参考电压2.5V，使用内部参考

	对SPI的时钟速度要求: 高达50MHz， 速度很快.
	SCLK下降沿读取数据, 每次传送24bit数据， 高位先传
*/

/* 定义GPIO端口 */
#define USE_HC574

/* 片选 */
#define DAC8562_RCC_CS 		RCC_AHB1Periph_GPIOG
#define DAC8562_PORT_CS		GPIOG
#define DAC8562_PIN_CS		GPIO_Pin_10

/* CLR */
#define DAC8562_RCC_CLR 	RCC_AHB1Periph_GPIOH
#define DAC8562_PORT_CLR	GPIOH
#define DAC8562_PIN_CLR		GPIO_Pin_7

/* LDAC */
#ifdef USE_HC574	/* 使用扩展IO */
	#define DAC8562_LDAC_0()	HC574_SetPin(NRF24L01_CE, 0);
	#define DAC8562_LDAC_1()	HC574_SetPin(NRF24L01_CE, 1);
#else
	#define DAC8562_RCC_LDAC 	RCC_AHB1Periph_GPIOA
	#define DAC8562_PORT_LDAC	GPIOA
	#define DAC8562_PIN_LDAC	GPIO_Pin_4

	#define DAC8562_LDAC_0()	DAC8562_PORT_LDAC->BSRRH = DAC8562_PIN_LDAC
	#define DAC8562_LDAC_1()	DAC8562_PORT_LDAC->BSRRL = DAC8562_PIN_LDAC
#endif

/* 定义口线置0和置1的宏 */
#define DAC8562_CS_0()	DAC8562_PORT_CS->BSRRH = DAC8562_PIN_CS
#define DAC8562_CS_1()	DAC8562_PORT_CS->BSRRL = DAC8562_PIN_CS

#define DAC8562_CLR_0()	DAC8562_PORT_CLR->BSRRH = DAC8562_PIN_CLR
#define DAC8562_CLR_1()	DAC8562_PORT_CLR->BSRRL = DAC8562_PIN_CLR

static void DAC8562_ConfigGPIO(void);
static void DAC8562_WriteCmd(uint32_t _cmd);

/* 定义电压和DAC值间的关系。 两点校准 x是dac y 是电压 0.1mV */
#define X1	0
#define Y1  -100000

#define X2	65535
#define Y2  100000

/*
*********************************************************************************************************
*	函 数 名: bsp_InitDAC8562
*	功能说明: 配置STM32的GPIO和SPI接口，用于连接 DAC8562
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitDAC8562(void)
{
	DAC8562_ConfigGPIO();
	
	DAC8562_CLR_0();		/* CLR接GND可靠一些，CLR是下降沿触发 */
	DAC8562_LDAC_0();		/* 不用异步更新模式，此引脚接GND */

	/* Power up DAC-A and DAC-B */
	DAC8562_WriteCmd((4 << 19) | (0 << 16) | (3 << 0));

	/* LDAC pin inactive for DAC-B and DAC-A  不使用LDAC引脚更新数据 */
	DAC8562_WriteCmd((6 << 19) | (0 << 16) | (3 << 0));

	/* 复位2个DAC到中间值, 输出0V */
	DAC8562_SetDacData(0, 32767);
	DAC8562_SetDacData(1, 32767);

	/* 选择内部参考并复位2个DAC的增益=2 （复位时，内部参考是禁止的) */
	DAC8562_WriteCmd((7 << 19) | (0 << 16) | (1 << 0));
}

/*
*********************************************************************************************************
*	函 数 名: DAC8562_SetCS(0)
*	功能说明: 设置CS。 用于运行中SPI共享。
*	形    参: 无
	返 回 值: 无
*********************************************************************************************************
*/
void DAC8562_SetCS(uint8_t _level)
{
	if (_level == 0)
	{
		bsp_SpiBusEnter();	/* 占用SPI总线， 用于总线共享 */

		#ifdef SOFT_SPI		/* 软件SPI */
			bsp_SetSpiSck(0);
			DAC8562_CS_0();
		#endif

		#ifdef HARD_SPI		/* 硬件SPI */
			bsp_SPI_Init(SPI_Direction_2Lines_FullDuplex | SPI_Mode_Master | SPI_DataSize_8b
				| SPI_CPOL_Low | SPI_CPHA_1Edge | SPI_NSS_Soft | SPI_BaudRatePrescaler_8 | SPI_FirstBit_MSB);
			DAC8562_CS_0();
		#endif
	}
	else
	{
		DAC8562_CS_1();

		bsp_SpiBusExit();	/* 释放SPI总线， 用于总线共享 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: DAC8562_ConfigGPIO
*	功能说明: 配置GPIO。 不包括 SCK  MOSI  MISO 共享的SPI总线。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DAC8562_ConfigGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(DAC8562_RCC_CS, ENABLE);

	DAC8562_CS_1();

	/* 配置几个推挽输出IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */

	/* SYNC */
	GPIO_InitStructure.GPIO_Pin = DAC8562_PIN_CS;
	GPIO_Init(DAC8562_PORT_CS, &GPIO_InitStructure);
	
	/* CLR */
	GPIO_InitStructure.GPIO_Pin = DAC8562_PIN_CLR;
	GPIO_Init(DAC8562_PORT_CLR, &GPIO_InitStructure);	
	
#ifdef USE_HC574	/* 使用扩展IO */
	;
#else
	/* LDAC */
	GPIO_InitStructure.GPIO_Pin = DAC8562_PIN_LDAC;
	GPIO_Init(DAC8562_PORT_LDAC, &GPIO_InitStructure);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: DAC8562_WriteCmd
*	功能说明: 向SPI总线发送24个bit数据。
*	形    参: _cmd : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
static void DAC8562_WriteCmd(uint32_t _cmd)
{
	DAC8562_SetCS(0);

	/*　DAC8562 SCLK时钟高达50M，因此可以不延迟 */

	/*　DAC8501 SCLK时钟高达30M，因此可以不延迟 */
	bsp_spiWrite0(_cmd >> 16);
	bsp_spiWrite0(_cmd >> 8);
	bsp_spiWrite0(_cmd);

	DAC8562_SetCS(1);
}

/*
*********************************************************************************************************
*	函 数 名: DAC8562_SetDacData
*	功能说明: 设置DAC输出，并立即更新。
*	形    参: _ch, 通道, 0 , 1
*		     _data : 数据
*	返 回 值: 无
*********************************************************************************************************
*/
void DAC8562_SetDacData(uint8_t _ch, uint16_t _dac)
{
	if (_ch == 0)
	{
		/* Write to DAC-A input register and update DAC-A; */
		DAC8562_WriteCmd((3 << 19) | (0 << 16) | (_dac << 0));
	}
	else if (_ch == 1)
	{
		/* Write to DAC-B input register and update DAC-A; */
		DAC8562_WriteCmd((3 << 19) | (1 << 16) | (_dac << 0));
	}
}


/*
*********************************************************************************************************
*	函 数 名: DAC8562_DacToVoltage
*	功能说明: 将DAC值换算为电压值，单位0.1mV
*	形    参: _dac  16位DAC字
*	返 回 值: 电压。单位0.1mV
*********************************************************************************************************
*/
int32_t DAC8562_DacToVoltage(uint16_t _dac)
{
	int32_t y;

	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	y =  CaculTwoPoint(X1, Y1, X2, Y2, _dac);
	return y;
}

/*
*********************************************************************************************************
*	函 数 名: DAC8562_VoltageToDac
*	功能说明: 将DAC值换算为电压值，单位 0.1mV
*	形    参: _volt 电压。单位0.1mV
*	返 回 值: 16位DAC字
*********************************************************************************************************
*/
uint32_t DAC8562_VoltageToDac(int32_t _volt)
{
	/* CaculTwoPoint(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x);*/
	return CaculTwoPoint(Y1, X1, Y2, X2, _volt);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
