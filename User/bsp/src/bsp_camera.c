/*
*********************************************************************************************************
*
*	ģ������ : ����ͷ����BSPģ��(For OV7670)
*	�ļ����� : bsp_camera.c
*	��    �� : V1.0
*	˵    �� : OV7670�������򡣱����������� guanfu_wang  ��OV7670����ͷ������FIFO,����LDO������24M����)
*			  ������STM32-V5�����弯����3.0V LDO��OV7670���磬���弯����24M��Դ�����ṩ������ͷ��
*
*			  ������ο��� guanfu_wang �ṩ�����ӡ�http://mcudiy.taobao.com/
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-03-01 armfly  ��ʽ����
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	������STM32-V5����������ͷ�ӿ�GPIO���壺 ��DCIM�豸�� ����ͷ  �� AD7606 ģ�鲻��ͬʱʹ�á�
	PA6/DCMI_PIXCLK
	PH8/DCMI_HSYNC
	PH9/DCMI_D0/AD7606_OS0
	PH10/DCMI_D1/AD7606_OS1
	PH11/DCMI_D2/AD7606_OS2
	PH12/DCMI_D3/AD7606_CONVST
	PH14/DCMI_D4/AD7606_RAGE
	PI4/DCMI_D5/AD7606_RESET
	PI5/DCMI_VSYNC
	PI6/DCMI_D6/AD7606_BUSY
	PI7/DCMI_D7/NRF905_CD

	--- I2C���߿�������ͷ
	PH4/I2C2_SCL
	PH5/I2C2_SDA

	������STM32-V6����������ͷ�ӿ�GPIO���壺 ��DCIM�豸�� ����ͷ  �ʹ���6, AD7606 ģ�鲻��ͬʱʹ�á�
	PA6/DCMI_PIXCLK
	PA4/DCMI_HSYNC/DAC_OUT1
	PC6/DCMI_D0/AD7606_CONVST
	PC7/DCMI_D1/USART6_RX
	PG10/DCMI_D2/NRF24L01_CSN
	PG11/DCMI_D3/ETH_RMII_TX_EN
	PE4/DCMI_D4/NRF24L01_IRQ
	PD3/DCMI_D5
	PE5/DCMI_D6/AD7606_BUSY
	PE6/DCMI_D7/NRF905_CD
	PB7/DCMI_VSYNC
	
	--- I2C���߿�������ͷ
	PB6/I2C2_SCL
	PB7/I2C2_SDA
*/

#define DCMI_DR_ADDRESS       	0x50050028
#define OV_REG_NUM  	116  //OV7670


/*��DMAͨ������,��ѡ���� DMA2_Stream1 +  DMA_Channel_1�� DMA2_Stream7 +  DMA_Channel_1  */

#define DMA_CLOCK              RCC_AHB1Periph_DMA2
#define DMA_STREAM             DMA2_Stream7
#define DMA_CHANNEL            DMA_Channel_1
#define DMA_IRQ                DMA2_Stream7_IRQn
#define DMA_IT_TCIF            DMA_IT_TCIF7
#define DMA_IRQHandler         DMA2_Stream7_IRQHandler


/* ���ֱ���

12 17 18 19 1A 03������

*/

/*
	����ΪOV7670 QVGA RGB565����  (by guanfu_wang)  http://mcudiy.taobao.com

	����RA8875ͼ��ģʽ�£�ɨ�跽��Ϊ�����ң����ϵ��¡�
	��wang_guanfu�ṩ��ȱʡֵ��ͬ����������ʵ��ĵ�����
*/
static const unsigned char  OV_reg[OV_REG_NUM][2] =
{
	{0x3a, 0x0c},
	{0x67, 0xc0},
	{0x68, 0x80},

	{0x40, 0xd0}, //RGB565
	//{0x40, 0x10}, //RGB565
	{0x12, 0x14}, //Output format, QVGA,RGB

	{0x32, 0x80},
	{0x17, 0x16},
	{0x18, 0x04},
	{0x19, 0x02},
	{0x1a, 0x7a},//0x7a,

	{0x03, 0x05},//0x0a,
	{0x0c, 0x00},
	{0x3e, 0x00},//

	{0x70, 0x00},
	{0x71, 0x01},
	{0x72, 0x11},
	{0x73, 0x00},//
	{0xa2, 0x02},
	{0x11, 0x01},

	{0x7a,  0x2C},
	{0x7b,  0x11},
	{0x7c,  0x1a},
	{0x7d,  0x2a},
	{0x7e,  0x42},
	{0x7f,  0x4c},
	{0x80,  0x56},
	{0x81,  0x5f},
	{0x82,  0x67},
	{0x83,  0x70},
	{0x84,  0x78},
	{0x85,  0x87},
	{0x86,  0x95},
	{0x87,  0xaf},
	{0x88,  0xc8},
	{0x89,  0xdf},

	////////////////
	{0x13, 0xe0},
	{0x00, 0x00},//AGC

	{0x10, 0x00},
	{0x0d, 0x00},
	{0x14, 0x10},//0x38, limit the max gain
	{0xa5, 0x05},
	{0xab, 0x07},

	{0x24, 0x75},//40
	{0x25, 0x63},
	{0x26, 0xA5},
	{0x9f, 0x78},
	{0xa0, 0x68},

	{0xa1, 0x03},//0x0b,
	{0xa6, 0xdf},//0xd8,
	{0xa7, 0xdf},//0xd8,
	{0xa8, 0xf0},
	{0xa9, 0x90},

	{0xaa, 0x94},//50
	{0x13, 0xe5},
	{0x0e, 0x61},
	{0x0f, 0x4b},
	{0x16, 0x02},

#if 1
	{0x1e, 0x37},//0x07, 0x17, 0x27, 0x37 ѡ��1��������ɨ�跽��. ��Ҫ��LCD��ɨ�跽��ƥ�䡣
#else
	{0x1e, 0x27},//0x07,
#endif


	{0x21, 0x02},
	{0x22, 0x91},
	{0x29, 0x07},
	{0x33, 0x0b},

	{0x35, 0x0b},//60
	{0x37, 0x1d},
	{0x38, 0x71},
	{0x39, 0x2a},
	{0x3c, 0x78},

	{0x4d, 0x40},
	{0x4e, 0x20},
	{0x69, 0x5d},
	{0x6b, 0x0a},//PLL
	{0x74, 0x19},
	{0x8d, 0x4f},

	{0x8e, 0x00},//70
	{0x8f, 0x00},
	{0x90, 0x00},
	{0x91, 0x00},
	{0x92, 0x00},//0x19,//0x66

	{0x96, 0x00},
	{0x9a, 0x80},
	{0xb0, 0x84},
	{0xb1, 0x0c},
	{0xb2, 0x0e},

	{0xb3, 0x82},//80
	{0xb8, 0x0a},
	{0x43, 0x14},
	{0x44, 0xf0},
	{0x45, 0x34},

	{0x46, 0x58},
	{0x47, 0x28},
	{0x48, 0x3a},
	{0x59, 0x88},
	{0x5a, 0x88},

	{0x5b, 0x44},//90
	{0x5c, 0x67},
	{0x5d, 0x49},
	{0x5e, 0x0e},
	{0x64, 0x04},
	{0x65, 0x20},

	{0x66, 0x05},
	{0x94, 0x04},
	{0x95, 0x08},
	{0x6c, 0x0a},
	{0x6d, 0x55},


	{0x4f, 0x80},
	{0x50, 0x80},
	{0x51, 0x00},
	{0x52, 0x22},
	{0x53, 0x5e},
	{0x54, 0x80},

	{0x76, 0xe1},

	{0x6e, 0x11},//100
	{0x6f, 0x9f},//0x9e for advance AWB
	{0x55, 0x00},//����
	{0x56, 0x40},//�Աȶ�
	{0x57, 0x80},//0x40,
};

static void CAM_ConfigCPU(void);
static uint8_t OV_InitReg(void);
static void OV_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue);
static uint8_t OV_ReadReg(uint8_t _ucRegAddr);

CAM_T g_tCam;

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitCamera
*	����˵��: ��������ͷGPIO��CAMERA�豸.
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitCamera(void)
{
	CAM_ConfigCPU();

	#if 1	/* ����Ĵ��룬��֤��д�Ĵ����Ƿ���ȷ */
	{
		uint8_t read;

		read = OV_ReadReg(0x3A);

		OV_WriteReg(0x3A, read + 1);

		read = OV_ReadReg(0x3A);

		OV_WriteReg(0x3A, read + 1);

		read = OV_ReadReg(0x3A);
	}
	#endif

	OV_InitReg();
}

/*
*********************************************************************************************************
*	�� �� ��: CAM_ConfigCPU
*	����˵��: ��������ͷGPIO��CAMERA�豸��0V7670��I2C�ӿ������� bsp_gpio_i2c.c �ļ�ʵ�֡�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void CAM_ConfigCPU(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;
  	DCMI_InitTypeDef DCMI_InitStructure;

	/* ʹ�� DCMI ʱ�� */
  	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);

	/* ʹ�� DMA2ʱ�� */
  	RCC_AHB1PeriphClockCmd(DMA_CLOCK, ENABLE);

  	/* ʹ�� DCMI �ӿ� GPIO ʱ�� */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC
		| RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);

 	/* ������STM32-V6�����������Դ�����ṩ24Mʱ�ӣ���˲���PA8����ʱ�� */
	#if 0	/* ����� PA8 ���24Mʱ�ӣ�����ʹ�����´���. */
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_MCO);//MCO1:PA8
	  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	  	GPIO_Init(GPIOA, &GPIO_InitStructure);
	    RCC_MCO2Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_3);
    #endif

	/*
		��DCMI��ص�GPIO����Ϊ����ģʽ  - STM32-V6
			PA6/DCMI_PIXCLK
			PA4/DCMI_HSYNC/DAC_OUT1
			PB7/DCMI_VSYNC			
			PC6/DCMI_D0/AD7606_CONVST
			PC7/DCMI_D1/USART6_RX
			PG10/DCMI_D2/NRF24L01_CSN
			PG11/DCMI_D3/ETH_RMII_TX_EN
			PE4/DCMI_D4/NRF24L01_IRQ
			PD3/DCMI_D5
			PE5/DCMI_D6/AD7606_BUSY
			PE6/DCMI_D7/NRF905_CD

	*/
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource10, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_DCMI);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_DCMI);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
	

	/* ���� DCMIC ���� */
  	DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_Continuous;
  	DCMI_InitStructure.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
  	DCMI_InitStructure.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising ;
  	DCMI_InitStructure.DCMI_VSPolarity = DCMI_VSPolarity_High;
  	DCMI_InitStructure.DCMI_HSPolarity = DCMI_HSPolarity_Low;
  	DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_1of2_Frame;		// DCMI_CaptureRate_All_Frame;
  	DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
  	DCMI_Init(&DCMI_InitStructure);
}

/*
*********************************************************************************************************
*	�� �� ��: OV_InitReg
*	����˵��: ��λOV7670, ����OV7670�ļĴ�����QVGA
*	��    ��: ��
*	�� �� ֵ: 0 ��ʾ��ȷ��1��ʾʧ��
*********************************************************************************************************
*/
static uint8_t OV_InitReg(void)
{
  	uint8_t i;

  	CAM_ConfigCPU();
	//bsp_InitI2C();		/* ����I2C����, �� bsp.c �ļ�ִ���� */

	OV_WriteReg(0x12, 0x80); 	/* Reset SCCB */

	bsp_DelayMS(5);

	/* ���� OV7670�Ĵ��� */
  	for (i = 0; i < OV_REG_NUM; i++)
  	{
		OV_WriteReg(OV_reg[i][0], OV_reg[i][1]);
  	}
	return 0;
}

/*
*********************************************************************************************************
*	�� �� ��: CAM_Start
*	����˵��: ����DMA��DCMI����ʼ����ͼ�����ݵ�LCD�Դ�
*	��    ��: _uiDispMemAddr �Դ��ַ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void CAM_Start(uint32_t _uiDispMemAddr)
{
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	memset((char *)_uiDispMemAddr, 0,  2000);
	
	/* ���� DMA2�� ��DCMI���ݼĴ���ֱ�Ӵ�����LCD�Դ� */
	//DMA_DeInit(DMA2_Stream1);
	DMA_DeInit(DMA_STREAM);
	DMA_InitStructure.DMA_Channel = DMA_CHANNEL;
	DMA_InitStructure.DMA_PeripheralBaseAddr = DCMI_DR_ADDRESS;		/* DCMI ���ݼĴ�����ַ */
	DMA_InitStructure.DMA_Memory0BaseAddr = _uiDispMemAddr;			/* LCD �Դ����ݵ�ַ */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 265*320*2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;			/* 429��Ҫ��ַ������RA8875���ܵ��� */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word; //DMA_MemoryDataSize_HalfWord;
	//DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	//DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA_STREAM, &DMA_InitStructure);

	/* Enable DMA Stream Transfer Complete interrupt */
	DMA_ITConfig(DMA_STREAM, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA_STREAM, ENABLE);

	/* Enable the DMA Stream IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DCMI_Cmd(ENABLE);
	DCMI_CaptureCmd(ENABLE);

	g_tCam.CaptureOk = 0;		/* ȫ�ֱ�־ */
}

/*
*********************************************************************************************************
*	�� �� ��: CAM_Stop
*	����˵��: ֹͣDMA��DCMI
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void CAM_Stop(void)
{
  	DMA_Cmd(DMA_STREAM, DISABLE);
  	DCMI_Cmd(DISABLE);
  	DCMI_CaptureCmd(DISABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: DMA2_Stream1_IRQHandler
*	����˵��: DMA��������жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void DMA_IRQHandler(void)
{	
	/* Test on DMA Stream Transfer Complete interrupt */
	if (DMA_GetITStatus(DMA_STREAM, DMA_IT_TCIF))
	{
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA_STREAM, DMA_IT_TCIF);  
		
		/* �ر����� */
		CAM_Stop();
		g_tCam.CaptureOk = 1;		/* ��ʾDMA������� */
	}	
}

#if 0  /* ��֪������ģ����š����� */
void OV_HW(unsigned int  hstart,unsigned int vstart,unsigned int hstop,unsigned int vstop)
{
	u8 v;
	OV_WriteReg(0x17,(hstart>>3)&0xff);//HSTART
	OV_WriteReg(0x18,(hstop>>3)&0xff);//HSTOP
	OV_ReadReg(0x32,&v);
	v=(v&0xc0)|((hstop&0x7)<<3)|(hstart&0x7);
	OV_WriteReg(0x32,v);//HREF

	OV_WriteReg(0x19,(vstart>>2)&0xff);//VSTART ��ʼ��8λ
	OV_WriteReg(0x1a,(vstop>>2)&0xff);//VSTOP	������8λ
	OV_ReadReg(0x03,&v);
	v=(v&0xf0)|((vstop&0x3)<<2)|(vstart&0x3);
	OV_WriteReg(0x03,v);//VREF
	OV_WriteReg(0x11,0x00);
}
#endif

/*
*********************************************************************************************************
*	�� �� ��: OV_WriteReg
*	����˵��: д0V7670�Ĵ���
*	��    ��: _ucRegAddr  : �Ĵ�����ַ
*			  _ucRegValue : �Ĵ���ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void OV_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue)
{
    i2c_Start();							/* ���߿�ʼ�ź� */

    i2c_SendByte(OV7670_SLAVE_ADDRESS);		/* �����豸��ַ+д�ź� */
	i2c_WaitAck();

    i2c_SendByte(_ucRegAddr);				/* ���ͼĴ�����ַ */
	i2c_WaitAck();

    i2c_SendByte(_ucRegValue);				/* ���ͼĴ�����ֵ */
	i2c_WaitAck();

    i2c_Stop();                   			/* ����ֹͣ�ź� */
}

/*
*********************************************************************************************************
*	�� �� ��: OV_ReadReg
*	����˵��: ��0V7670�Ĵ���
*	��    ��: _ucRegAddr  : �Ĵ�����ַ
*	�� �� ֵ: �Ĵ���ֵ
*********************************************************************************************************
*/
static uint8_t OV_ReadReg(uint8_t _ucRegAddr)
{
	uint16_t usRegValue;

	i2c_Start();                  			/* ���߿�ʼ�ź� */
	i2c_SendByte(OV7670_SLAVE_ADDRESS);		/* �����豸��ַ+д�ź� */
	i2c_WaitAck();
	i2c_SendByte(_ucRegAddr);				/* ���͵�ַ */
	i2c_WaitAck();

	i2c_Stop();			/* 0V7670 ��Ҫ���� stop, �����ȡ�Ĵ���ʧ�� */

	i2c_Start();                  			/* ���߿�ʼ�ź� */
	i2c_SendByte(OV7670_SLAVE_ADDRESS + 1);/* �����豸��ַ+���ź� */
	i2c_WaitAck();

	usRegValue = i2c_ReadByte();       		/* �������ֽ����� */
	i2c_NAck();
	i2c_Stop();                  			/* ����ֹͣ�ź� */

	return usRegValue;
}

/*
*********************************************************************************************************
*	�� �� ��: OV_ReadID
*	����˵��: ��0V7670��оƬID
*	��    ��: ��
*	�� �� ֵ: оƬID. ����Ӧ�÷��� 0x7673
*********************************************************************************************************
*/
uint16_t OV_ReadID(void)
{
	uint8_t idh,idl;

	idh = OV_ReadReg(0x0A);
	idl = OV_ReadReg(0x0B);
	return (idh << 8) + idl;
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
