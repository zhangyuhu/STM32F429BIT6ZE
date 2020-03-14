/*
*********************************************************************************************************
*
*   模块名称 : 蜂鸣器驱动模块
*   文件名称 : bsp_beep.c
*   版    本 : V1.1
*   说    明 : 驱动蜂鸣器.
*
*   修改记录 :
*       版本号  日期        作者     说明
*       V1.0    2014-10-20 armfly  正式发布
*       V1.1    2015-10-06 armfly  增加静音函数。用于临时屏蔽蜂鸣器发声。
*
*   Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "log.h"

/* Macro definition */

//#define BEEP_HAVE_POWER       /* 定义此行表示有源蜂鸣器，直接通过GPIO驱动, 无需PWM */

#ifdef  BEEP_HAVE_POWER     /* 有源蜂鸣器 */

/* PA8 */
#define GPIO_RCC_BEEP   RCC_AHB1Periph_GPIOA
#define GPIO_PORT_BEEP  GPIOA
#define GPIO_PIN_BEEP   GPIO_Pin_8

#define BEEP_ENABLE()   GPIO_PORT_BEEP->BSRRL = GPIO_PIN_BEEP   /* 使能蜂鸣器鸣叫 */
#define BEEP_DISABLE()  GPIO_PORT_BEEP->BSRRH = GPIO_PIN_BEEP   /* 禁止蜂鸣器鸣叫 */
#else   /* 无源蜂鸣器 */
/* PA8 ---> TIM1_CH1 */

/* 1500表示频率1.5KHz，5000表示50.00%的占空比 */
#define BEEP_ENABLE()   bsp_SetTIMOutPWM(GPIOA, GPIO_Pin_8, TIM1, 1, 1500, 5000);

/* 设置频率和占空比 */
#define BEEP_SET_FRQ_DUTY_CYCLE(pwm_freq,duty_cycle_percent)   bsp_SetTIMOutPWM(GPIOA, GPIO_Pin_8, TIM1, 1, pwm_freq, duty_cycle_percent*100);

/* 禁止蜂鸣器鸣叫 */
#define BEEP_DISABLE()  bsp_SetTIMOutPWM(GPIOA, GPIO_Pin_8, TIM1, 1, 1500, 0);
#endif

#define BEEP_TIMER_CREATE() \
    SW_TIMER_CREATE(SW_TIMER_ID_BEEP_DELAY, SW_TIMER_TYPE_SINGLE, beep_timeout_handler)
#define BEEP_TIMER_START(timeout, data) \
    SW_TIMER_START_WITH_DATA(SW_TIMER_ID_BEEP_DELAY, timeout, data)
#define BEEP_TIMER_STOP() SW_TIMER_STOP(SW_TIMER_ID_BEEP_DELAY)

/* Global Variables */
#define BEEP_LOW_1 (262)
#define BEEP_LOW_2 (294)
#define BEEP_LOW_3 (330)
#define BEEP_LOW_4 (349)
#define BEEP_LOW_5 (392)
#define BEEP_LOW_6 (440)
#define BEEP_LOW_7 (494)

#define BEEP_MID_1 (523)
#define BEEP_MID_2 (578)
#define BEEP_MID_3 (659)
#define BEEP_MID_4 (698)
#define BEEP_MID_5 (784)
#define BEEP_MID_6 (880)
#define BEEP_MID_7 (988)

#define BEEP_HGH_1 (1046)
#define BEEP_HGH_2 (1175)
#define BEEP_HGH_3 (1318)
#define BEEP_HGH_4 (1397)
#define BEEP_HGH_5 (1568)
#define BEEP_HGH_6 (1760)
#define BEEP_HGH_7 (1976)

#define BEEP_2500 (BEEP_HGH_1)
#define BEEP_3200 (BEEP_HGH_1)
#define BEEP_2900 (2900)
#define BEEP_2600 (2600)
#define BEEP_2300 (2300)

#define BZ_MS(para) (para)

const int BEEP_RAISE[] = {
    BEEP_MID_3, BZ_MS(125), BEEP_MID_4, BZ_MS(125), BEEP_MID_5, BZ_MS(125), 0, -1,
};

const int BEEP_RAISE_LOW[] = {
    BEEP_LOW_3, BZ_MS(300), BEEP_LOW_4, BZ_MS(300), BEEP_LOW_5, BZ_MS(300), 0, -1,
};

const int BEEP_REDUCE[] = {
    BEEP_MID_5, BZ_MS(125), BEEP_MID_3, BZ_MS(125), BEEP_MID_1, BZ_MS(125), 0, -1,
};

const int BEEP_RAISE_ALL[] = {
    BEEP_MID_1, BZ_MS(125), 0,          BZ_MS(5),   BEEP_MID_2, BZ_MS(125), 0,
    BZ_MS(5),   BEEP_MID_3, BZ_MS(125), 0,          BZ_MS(5),   BEEP_MID_4, BZ_MS(125),
    0,          BZ_MS(5),   BEEP_MID_5, BZ_MS(125), 0,          BZ_MS(5),   BEEP_MID_6,
    BZ_MS(125), 0,          BZ_MS(5),   BEEP_MID_7, BZ_MS(125), 0,          -1,
};

const int BEEP_REDUCE_ALL[] = {
    BEEP_MID_7, BZ_MS(125), 0,          BZ_MS(5),   BEEP_MID_6, BZ_MS(125), 0,
    BZ_MS(5),   BEEP_MID_5, BZ_MS(125), 0,          BZ_MS(5),   BEEP_MID_4, BZ_MS(125),
    0,          BZ_MS(5),   BEEP_MID_3, BZ_MS(125), 0,          BZ_MS(5),   BEEP_MID_2,
    BZ_MS(125), 0,          BZ_MS(5),   BEEP_MID_1, BZ_MS(125), 0,          -1,
};

const int BEEP_DI[] = {
    BEEP_3200,
    BZ_MS(125),
    0,
    -1,
};

const int BEEP_DIDI[] = {
    BEEP_3200, BZ_MS(125), 0, BZ_MS(125), BEEP_3200, BZ_MS(125), 0, -1,
};

const int DELAY_AND_BEEP_DIDI[] = {
    0, BZ_MS(300), BEEP_3200, BZ_MS(125), 0, BZ_MS(125), BEEP_3200, BZ_MS(125), 0, -1,
};

const int BEEP_FORBID[] = {
    BEEP_2500, BZ_MS(125), 0, BZ_MS(125), BEEP_2500, BZ_MS(125), 0, BZ_MS(125),
    BEEP_2500, BZ_MS(125), 0, BZ_MS(125), BEEP_2500, BZ_MS(125), 0, -1,
};

const int BEEP_STOP[] = {
    BEEP_LOW_1,
    BZ_MS(600),
    0,
    -1,
};

const int BEEP_2S[] = {
    BEEP_3200,
    BZ_MS(1000),
    0,
    -1,
};

const int BEEP_ALARM_5S[] = {
    BEEP_3200,  BZ_MS(520), 0,          BZ_MS(315), BEEP_3200,  BZ_MS(520), 0,
    BZ_MS(315), BEEP_3200,  BZ_MS(520), 0,          BZ_MS(315), BEEP_3200,  BZ_MS(520),
    0,          BZ_MS(315), BEEP_3200,  BZ_MS(520), 0,          BZ_MS(315), BEEP_3200,
    BZ_MS(520), 0,          BZ_MS(315), 0,          -1,
};

/* Local Variables */

static int m_sample_level;
static int m_capture_time;
static const int* m_music_index      = NULL;
static volatile uint8_t m_beep_level = BEEP_LEVEL_NONE;

static volatile bool is_beep_driving_flag        = false;
static volatile volume_level_t used_volume_level = nomal_level;

/* Internal functions declaration */
static void beep_timeout_handler(void* p_context);


/*
*********************************************************************************************************
*   函 数 名: BEEP_InitHard
*   功能说明: 初始化蜂鸣器硬件
*   形    参: 无
*   返 回 值: 无
*********************************************************************************************************
*/
static void BEEP_InitHard(void)
{
#ifdef  BEEP_HAVE_POWER     /* 有源蜂鸣器 */
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 打开GPIOF的时钟 */
    RCC_AHB1PeriphClockCmd(GPIO_RCC_BEEP, ENABLE);

    BEEP_DISABLE();

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       /* 设为输出口 */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      /* 设为推挽模式 */
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    /* 上下拉电阻不使能 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   /* IO口最大速度 */

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_BEEP;
    GPIO_Init(GPIO_PORT_BEEP, &GPIO_InitStructure);
#endif
}


/* Public functions */

/************************************************************
 * @brief beep_init
 ************************************************************/
int beep_init(void) {

    BEEP_InitHard();
    BEEP_TIMER_CREATE();

    m_beep_level = BEEP_LEVEL_NONE;

    return 0;
}

/************************************************************
 * @brief beep_shutdown
 ************************************************************/
int beep_shutdown(void) {
    BEEP_TIMER_STOP();
    BEEP_DISABLE();


    m_beep_level   = BEEP_LEVEL_NONE;
    m_sample_level = 0;
    m_capture_time = 0;
    m_music_index  = NULL;

    return 0;
}

/************************************************************
 * @brief beep_start
 ************************************************************/
int beep_start(int level, const int beep_array[]) {
    LOG("beep_start %d - %d", level, m_beep_level);

    if (level > m_beep_level)
        return -1;

    beep_shutdown();

    m_beep_level = level;

    m_music_index = beep_array;

    BEEP_TIMER_START(10, NULL);

    return 0;
}

/************************************************************
 * @brief beep_start_bass
 ************************************************************/
void beep_start_bass(int level, const int beep_array[]) {
    // LOG("beep_start_bass %d - %d",level,m_beep_level);


    if (level > m_beep_level)
        return;

    beep_shutdown();

    m_beep_level = level;

    m_music_index = beep_array;

    BEEP_TIMER_START(10, (void*)&used_volume_level);
}

/************************************************************
 * @brief get_beep_volume_level
 ************************************************************/
volume_level_t get_beep_volume_level(void) {
    return used_volume_level;
}

/************************************************************
 * @brief set_beep_volume_level
 ************************************************************/
void set_beep_volume_level(volume_level_t level) {
    used_volume_level = level;
}

/************************************************************
 * @brief check_beep_driving_flag
 ************************************************************/
bool check_beep_driving_flag(void) {
    return is_beep_driving_flag;
}

/************************************************************
 * @brief beep_timeout_handler
 ************************************************************/
static void beep_timeout_handler(void* p_context) {
    // LOG("beep_timeout_handler");
    static bool beep_stop_flag = false;
    is_beep_driving_flag       = true;

    if (m_music_index != NULL) {
        m_sample_level = *(m_music_index);
        m_capture_time = *(m_music_index + 1);

        if ((m_sample_level == 0) && (m_capture_time < 0)) {
            beep_stop_flag = true;
        } else {
            if (m_sample_level != 0) {
                if ((p_context != NULL) && (bass_level == *((volume_level_t*)p_context))) {
                    BEEP_SET_FRQ_DUTY_CYCLE((uint32_t)(m_sample_level), (uint8_t)2);
                } else {
                    BEEP_SET_FRQ_DUTY_CYCLE((uint32_t)(m_sample_level), (uint8_t)50);
                }
            } else {
                BEEP_DISABLE();
            }

            BEEP_TIMER_START((m_capture_time + 5), p_context);

            m_music_index += 2;
        }
    } else {
        beep_stop_flag = true;
    }

    if (beep_stop_flag == true) {
        beep_stop_flag       = false;
        is_beep_driving_flag = false;
        beep_shutdown();
    }
}

void beep_test(void) {
    beep_init();

    beep_start(BEEP_LEVEL_NORMAL, BEEP_DI);
    bsp_DelayMS(3000);
    beep_start(BEEP_LEVEL_NORMAL, BEEP_DIDI);
    bsp_DelayMS(5000);
    beep_start(BEEP_LEVEL_NORMAL, BEEP_RAISE);
    bsp_DelayMS(5000);
    beep_start(BEEP_LEVEL_NORMAL, BEEP_RAISE_LOW);
    bsp_DelayMS(5000);
    beep_start(BEEP_LEVEL_NORMAL, BEEP_REDUCE);
    bsp_DelayMS(5000);
    beep_start(BEEP_LEVEL_NORMAL, BEEP_REDUCE_ALL);
    bsp_DelayMS(5000);

    beep_start(BEEP_LEVEL_NORMAL, BEEP_RAISE_ALL);
    bsp_DelayMS(5000);
    beep_start(BEEP_LEVEL_NORMAL, DELAY_AND_BEEP_DIDI);
    bsp_DelayMS(5000);
    beep_start(BEEP_LEVEL_NORMAL, BEEP_FORBID);
    bsp_DelayMS(5000);
    beep_start(BEEP_LEVEL_NORMAL, BEEP_STOP);
    bsp_DelayMS(5000);

    beep_start(BEEP_LEVEL_NORMAL, BEEP_2S);
    bsp_DelayMS(5000);
    beep_start(BEEP_LEVEL_NORMAL, BEEP_ALARM_5S);
    bsp_DelayMS(5000);
}

