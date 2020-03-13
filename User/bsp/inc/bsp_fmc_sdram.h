/*
*********************************************************************************************************
*
*	ģ������ : �ⲿSDRAM����ģ��
*	�ļ����� : bsp_fmc_sdram.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2014-05-04 armfly  ST�̼���汾 V1.3.0
*
*	Copyright (C), 2014-2015, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_FMC_SDRAM_H
#define _BSP_FMC_SDRAM_H

#define EXT_SDRAM_ADDR  	((uint32_t)0xC0000000)
#define EXT_SDRAM_SIZE		(16 * 1024 * 1024)

/* LCD�Դ�,��1ҳ, ����2M�ֽ� */
#define SDRAM_LCD_BUF1 		EXT_SDRAM_ADDR

/* LCD�Դ�,��2ҳ, ����2M�ֽ� */
#define SDRAM_LCD_BUF2		(EXT_SDRAM_ADDR + 2 * 1024 * 1024)

/* ʣ�µ�12M�ֽڣ��ṩ��Ӧ�ó���ʹ�� */
#define SDRAM_APP_BUF		(EXT_SDRAM_ADDR + 4 * 1024 * 1024)
#define SDRAM_APP_SIZE		(12 * 1024 * 1024)

void bsp_InitExtSDRAM(void);
uint32_t bsp_TestExtSDRAM1(void);
uint32_t bsp_TestExtSDRAM2(void);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
