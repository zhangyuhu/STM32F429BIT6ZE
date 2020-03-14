#include "bsp.h"            /* 底层硬件驱动 */
#include "includes.h"

#include "module_config.h"


/* 仅允许本文件内调用的函数声明 */
static void PrintfInfo(void);

/*
*********************************************************************************************************
*   函 数 名: PrintfInfo
*   功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*   形    参：无
*   返 回 值: 无
*********************************************************************************************************
*/
static void PrintfInfo(void)
{
    /* 检测CPU ID */
    {
        /* 参考手册：
            32.6.1 MCU device ID code
            33.1 Unique device ID register (96 bits)
        */
        uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;

        CPU_Sn0 = *(__IO uint32_t*)(0x1FFF7A10);
        CPU_Sn1 = *(__IO uint32_t*)(0x1FFF7A10 + 4);
        CPU_Sn2 = *(__IO uint32_t*)(0x1FFF7A10 + 8);

        LOG("\r\nCPU : STM32F429BI, LQFP208, 主频: %dMHz", SystemCoreClock / 1000000);
        LOG("UID = %08X %08X %08X", CPU_Sn2, CPU_Sn1, CPU_Sn0);
    }

    /* 打印ST固件库版本，这3个定义宏在stm32f10x.h文件中 */
    LOG("* 固件库版本 : V%d.%d.%d (STM32F4xx_StdPeriph_Driver)", __STM32F4XX_STDPERIPH_VERSION_MAIN,
            __STM32F4XX_STDPERIPH_VERSION_SUB1,__STM32F4XX_STDPERIPH_VERSION_SUB2);
}

/*
*********************************************************************************************************
*   函 数 名: main
*   功能说明: c程序入口
*   形    参：无
*   返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
    /*
        在启动调度前，为了防止初始化STM32外设时有中断服务程序执行，这里禁止全局中断(除了NMI和HardFault)。
        这样做的好处是：
        1. 防止执行的中断服务程序中有FreeRTOS的API函数。
        2. 保证系统正常启动，不受别的中断影响。
        3. 关于是否关闭全局中断，大家根据自己的实际情况设置即可。
            在移植文件port.c中的函数prvStartFirstTask中会重新开启全局中断。通过指令cpsie i开启，__set_PRIMASK(1)
            和cpsie i是等效的。
    */
    __set_PRIMASK(1);

    /*
    ST固件库中的启动文件已经执行了 SystemInit() 函数，该函数在 system_stm32f4xx.c 文件，主要功能是
    配置CPU系统的时钟，内部Flash访问时序，配置FSMC用于外部SRAM
    */

    /* 硬件初始化 */
    bsp_Init();

    PrintfInfo();   /* 打印例程信息到串口1 */

    /* 1. 初始化一个定时器中断，精度高于滴答定时器中断，这样才可以获得准确的系统信息 仅供调试目的，实际项
        目中不要使用，因为这个功能比较影响系统实时性。
    2. 为了正确获取FreeRTOS的调试信息，可以考虑将上面的关闭中断指令__set_PRIMASK(1); 注释掉。
    */
    vSetupSysInfoTest();

    /* 创建任务 */
    AppTaskCreate();

    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();

    /*
        如果系统正常启动是不会运行到这里的，运行到这里极有可能是用于定时器任务或者空闲任务的
        heap空间不足造成创建失败，此要加大FreeRTOSConfig.h文件中定义的heap大小：
        #define configTOTAL_HEAP_SIZE       ( ( size_t ) ( 17 * 1024 ) )
    */
    while(1);

}



