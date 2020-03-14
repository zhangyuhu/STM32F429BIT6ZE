#include "includes.h"


/*
**********************************************************************************************************
                                            函数声明
**********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskSocket(void *pvParameters);
static void vTaskTCPnet(void *pvParameters);
static void vTaskTest(void *pvParameters);
static void vTaskStart(void *pvParameters);


/*
**********************************************************************************************************
                                            变量声明
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskTest = NULL;

static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static TaskHandle_t xHandleTaskSocket = NULL;
static TaskHandle_t xHandleTaskTCPnet = NULL;
static TaskHandle_t xHandleTaskStart = NULL;
EventGroupHandle_t xCreatedEventGroup = NULL;


/*
*********************************************************************************************************
*   函 数 名: vTaskTaskUserIF
*   功能说明: 接口消息处理。
*   形    参: pvParameters 是在创建该任务时传递的形参
*   返 回 值: 无
*   优 先 级: 1  (数值越小优先级越低，这个跟uCOS相反)
*********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters)
{
    uint8_t ucKeyCode;
    uint8_t pcWriteBuffer[500];


    while(1)
    {
        ucKeyCode = bsp_GetKey();

        if (ucKeyCode != KEY_NONE)
        {
            switch (ucKeyCode)
            {
                /* K1键按下 */
                case KEY_DOWN_K1:
                    App_Printf("K1键按下\r\n");
                    break;

                /* K2键按下 */
                case KEY_DOWN_K2:
                    App_Printf("K2键按下\r\n");
                    break;

                /* K3键按下 */
                case KEY_DOWN_K3:
                    App_Printf("K3键按下\r\n");
                    break;

                /* 摇杆的OK键按下，打印任务执行情况 */
                case JOY_DOWN_OK:
                    App_Printf("=================================================\r\n");
                    App_Printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
                    vTaskList((char *)&pcWriteBuffer);
                    App_Printf("%s\r\n", pcWriteBuffer);

                    App_Printf("\r\n任务名       运行计数         使用率\r\n");
                    vTaskGetRunTimeStats((char *)&pcWriteBuffer);
                    App_Printf("%s\r\n", pcWriteBuffer);
                    App_Printf("当前动态内存剩余大小 = %d字节\r\n", xPortGetFreeHeapSize());
                    break;

                /* 其他的键值不处理 */
                default:
                    break;
            }
        }

        vTaskDelay(20);
    }
}

/*
*********************************************************************************************************
*   函 数 名: vTaskLED
*   功能说明: LED闪烁
*   形    参: pvParameters 是在创建该任务时传递的形参
*   返 回 值: 无
*   优 先 级: 2
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 500;

    /* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
        bsp_LedToggle(2);

        /* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*
*********************************************************************************************************
*   函 数 名: vTaskMsgPro
*   功能说明: 消息处理，这里用作按键检测
*   形    参: pvParameters 是在创建该任务时传递的形参
*   返 回 值: 无
*   优 先 级: 3
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
    while(1)
    {
        /* 按键扫描 */
        bsp_KeyScan();
        vTaskDelay(10);
    }
}

/*
*********************************************************************************************************
*   函 数 名: vTaskSocket
*   功能说明: RL-TCPnet网络测试任务
*   形    参: pvParameters 是在创建该任务时传递的形参
*   返 回 值: 无
*   优 先 级: 4
*********************************************************************************************************
*/
static void vTaskSocket(void *pvParameters)
{
    while(1)
    {
//        TCPnetTest();
    }
}

/*
*********************************************************************************************************
*   函 数 名: vTaskTCPnet
*   功能说明: RL-TCPnet网络主任务
*   形    参: pvParameters 是在创建该任务时传递的形参
*   返 回 值: 无
*   优 先 级: 5
*********************************************************************************************************
*/
static void vTaskTCPnet(void *pvParameters)
{
    while(1)
    {
        /* RL-TCPnet处理函数 */
//        main_TcpNet();
        vTaskDelay(2);
    }
}

/*
*********************************************************************************************************
*   函 数 名: vTaskStart
*   功能说明: 启动任务，也是最高优先级任务，这里实现RL-TCPnet的时间基准更新
*   形    参: pvParameters 是在创建该任务时传递的形参
*   返 回 值: 无
*   优 先 级: 6
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 100;

    /* 初始化RL-TCPnet */
//    init_TcpNet ();

    /* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
        /* RL-TCPnet时间基准更新函数 */
//        timer_tick ();

        /* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}


/******************************************************************************
 * @brief vTaskTest
 *****************************************************************************/
static void vTaskTest(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 100;

    /* 获取当前的系统时间 */
    xLastWakeTime = xTaskGetTickCount();

    LOG("vTaskTest %d",xLastWakeTime);

    sw_timer_test();

#if (1 == NAND_FATFS_TEST)
    DemoFatFS();    /* NAND Flash文件系统演示程序 */
#endif

#if (1 == NAND_SPIFS_TEST)
    DemoSPIFS();
#endif

#if (1 == BEEP_TEST)
    beep_test();
#endif

    while(1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}


/*
*********************************************************************************************************
*   函 数 名: AppTaskCreate
*   功能说明: 创建应用任务
*   形    参：无
*   返 回 值: 无
*********************************************************************************************************
*/
void AppTaskCreate (void)
{
    xTaskCreate( vTaskTaskUserIF,       /* 任务函数  */
                 "vTaskUserIF",         /* 任务名    */
                 512,                   /* 任务栈大小，单位word，也就是4字节 */
                 NULL,                  /* 任务参数  */
                 1,                     /* 任务优先级*/
                 &xHandleTaskUserIF );  /* 任务句柄  */


    xTaskCreate( vTaskLED,          /* 任务函数  */
                 "vTaskLED",        /* 任务名    */
                 512,               /* stack大小，单位word，也就是4字节 */
                 NULL,              /* 任务参数  */
                 2,                 /* 任务优先级*/
                 &xHandleTaskLED ); /* 任务句柄  */

    xTaskCreate( vTaskMsgPro,           /* 任务函数  */
                 "vTaskMsgPro",         /* 任务名    */
                 512,                   /* 任务栈大小，单位word，也就是4字节 */
                 NULL,                  /* 任务参数  */
                 3,                     /* 任务优先级*/
                 &xHandleTaskMsgPro );  /* 任务句柄  */

    xTaskCreate( vTaskSocket,           /* 任务函数  */
                 "vTaskSocket",         /* 任务名    */
                 512,                   /* 任务栈大小，单位word，也就是4字节 */
                 NULL,                  /* 任务参数  */
                 4,                     /* 任务优先级*/
                 &xHandleTaskSocket );  /* 任务句柄  */

    xTaskCreate( vTaskTCPnet,           /* 任务函数  */
                 "vTaskTCPnet",         /* 任务名    */
                 512,                   /* 任务栈大小，单位word，也就是4字节 */
                 NULL,                  /* 任务参数  */
                 5,                     /* 任务优先级*/
                 &xHandleTaskTCPnet );  /* 任务句柄  */

    xTaskCreate( vTaskTest,            /* 任务函数  */
                 "vTaskTest",          /* 任务名    */
                 512,                   /* 任务栈大小，单位word，也就是4字节 */
                 NULL,                  /* 任务参数  */
                 6,                     /* 任务优先级*/
                 &xHandleTaskTest );   /* 任务句柄  */

    xTaskCreate( vTaskStart,            /* 任务函数  */
                 "vTaskStart",          /* 任务名    */
                 512,                   /* 任务栈大小，单位word，也就是4字节 */
                 NULL,                  /* 任务参数  */
                 6,                     /* 任务优先级*/
                 &xHandleTaskStart );   /* 任务句柄  */


}


