#ifndef __MODULE_CONFIG_H
#define __MODULE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define NAND_FATFS_TEST			(0)
#define NAND_SPIFS_TEST			(1)
	
#if (1 == NAND_FATFS_TEST)
#include "demo_nand_fatfs.h"
#endif

#if (1 == NAND_SPIFS_TEST)
#include "demo_spiflash_spifs.h"
#endif
	
#ifdef __cplusplus
}
#endif

#endif
