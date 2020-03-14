#include "bsp.h"

#include "spiffs.h"
#include "demo_spiflash_spifs.h"


spiffs SPIFlashFS;
static u8_t FS_Work_Buf[SPIFLASH_CFG_LOG_PAGE_SZ*2];
static u8_t FS_FDS[32*4];
static u8_t FS_Cache_Buf[(SPIFLASH_CFG_LOG_PAGE_SZ+32)*4];


static s32_t spiflash_spiffs_read(u32_t addr, u32_t size, u8_t *dst)
{
    sf_ReadBuffer(dst,addr,size);
    return 0;
}

static s32_t spiflash_spiffs_write(u32_t addr, u32_t size, u8_t *src)
{
    sf_WriteBuffer(src,addr,size);
    return 0;
}

static s32_t spiflash_spiffs_erase(u32_t addr, u32_t size)
{
    sf_EraseSector(addr);
    return 0;
}
 /***********************************************
 *   加载SPI FLASH文件系统
 */
static void Mount_SPI_Flash_File_System(void)
{
    spiffs_config SPIFlashCfg;
    SPIFlashCfg.phys_size        = SPIFLASH_CFG_PHYS_SZ;         // SPI Flash的总容量
    SPIFlashCfg.phys_addr        = SPIFLASH_CFG_PHYS_ADDR;       // 起始地址
    SPIFlashCfg.phys_erase_block = SPIFLASH_CFG_PHYS_ERASE_SZ;   // FLASH最大可擦除的块大小(W25X64可以按扇区擦除(4K)或者按块擦除(64K))
    SPIFlashCfg.log_block_size   = SPIFLASH_CFG_LOG_BLOCK_SZ;   // 块的大小(W25X64每块包含16个扇区，每个扇区4K字节，每块的总容量为：16X4=64K=65535字节)
    SPIFlashCfg.log_page_size    = SPIFLASH_CFG_LOG_PAGE_SZ;    //  (W25X64每页包含256个字节，16个页构成一个扇区)

    SPIFlashCfg.hal_read_f =  spiflash_spiffs_read;    //读
    SPIFlashCfg.hal_write_f = spiflash_spiffs_write;   //写
    SPIFlashCfg.hal_erase_f = spiflash_spiffs_erase;   //擦除

    //挂载SPIFS
    int res = SPIFFS_mount(&SPIFlashFS,
                            &SPIFlashCfg,
                            FS_Work_Buf,
                            FS_FDS,
                            sizeof(FS_FDS),
                            FS_Cache_Buf,
                            sizeof(FS_Cache_Buf),
                            0);

}




char WriteBuf[]={"Hi,Budy! if you get this Message......Congratulations!You have succeeded!!\r\n"};
char ReadBuf[80];

void DemoSPIFS(void)
{
    Mount_SPI_Flash_File_System();

    //打开文件，如果文件不存在，自动创建
    spiffs_file fd = SPIFFS_open(&SPIFlashFS, "my_file", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);

    //写文件
    if (SPIFFS_write(&SPIFlashFS, fd, WriteBuf, sizeof(WriteBuf)) < 0)
        printf("errno %i\n", SPIFFS_errno(&SPIFlashFS));

    SPIFFS_close(&SPIFlashFS, fd);

    //读文件
    fd = SPIFFS_open(&SPIFlashFS, "my_file", SPIFFS_RDWR, 0);

    if (SPIFFS_read(&SPIFlashFS, fd, ReadBuf, sizeof(WriteBuf)) < 0)
        printf("errno %i\n", SPIFFS_errno(&SPIFlashFS));

    SPIFFS_close(&SPIFlashFS, fd);

    //打印输出文件内容
    printf("%s\n", ReadBuf);
}

