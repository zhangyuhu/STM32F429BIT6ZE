/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"			/* FatFs lower layer API */
#include "bsp.h"			/* 底层驱动， 提供SD, USB, NAND 驱动 */
//#include "usbh_bsp_msc.h"	/* 提供U盘的读写函数 */

//#define ff_printf	printf
#define ff_printf(...)

#define SECTOR_SIZE		512	/* SD卡扇区大小必须为512 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv)
	{
	case FS_SD :
		stat = 0;
		return stat;

	case FS_NAND :
		stat = 0;
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;

	switch (pdrv)
	{
		case FS_SD :		/* SD卡 */
			stat = STA_NOINIT;
			return stat;

		case FS_NAND :		/* NAND Flash */
			if (NAND_Init() == NAND_OK)
			{
				stat = RES_OK;
			}
			else
			{
				/* 如果初始化失败，请执行低级格式化 */
				printf("NAND_Init() Error!  \r\n");
				stat = RES_ERROR;
			}
			return stat;

		default :
			break;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;

	switch (pdrv) {
	case FS_SD :
		res = RES_OK;
		return res;

	case FS_NAND :
		if (NAND_OK == NAND_ReadMultiSectors(buff, sector, 512, count))
		{
			res = RES_OK;
		}
		else
		{
			printf("NAND_ReadMultiSectors() Error! sector = %d, count = %d \r\n", sector, count);
			res = RES_ERROR;
		}
		return res;

	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;

	switch (pdrv) {
	case FS_SD :
		return RES_OK;

	case FS_NAND :
		if (NAND_OK == NAND_WriteMultiSectors((uint8_t *)buff, sector, 512, count))
		{
			res = RES_OK;
		}
		else
		{
			printf("NAND_ReadMultiSectors() Error! sector = %d, count = %d \r\n", sector, count);
			res = RES_ERROR;
		}
		return res;
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
		DRESULT res;

	switch (pdrv) {
	case FS_SD :
		res = RES_OK;
		return res;

	case FS_NAND :
		res = RES_OK;
		return res;
	}
	return RES_PARERR;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: get_fattime
*	功能说明: 获得系统时间，用于改写文件的创建和修改时间。客户可以自行移植和系统的RTC关联起来
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
DWORD get_fattime (void)
{
	/* 如果有全局时钟，可按下面的格式进行时钟转换. 这个例子是2014-07-02 00:00:00 */

	return	  ((DWORD)(2014 - 1980) << 25)	/* Year = 2013 */
			| ((DWORD)7 << 21)				/* Month = 1 */
			| ((DWORD)2 << 16)				/* Day_m = 1*/
			| ((DWORD)0 << 11)				/* Hour = 0 */
			| ((DWORD)0 << 5)				/* Min = 0 */
			| ((DWORD)0 >> 1);				/* Sec = 0 */
}


