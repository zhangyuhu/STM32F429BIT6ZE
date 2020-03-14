#include "global.h"
#include "drv_timer.h"


#ifndef RELEASE_VERSION_ENABLE
#define APP_TIMER_LOG_EN (0)
#else
#define APP_TIMER_LOG_EN (0)
#endif

#if (APP_TIMER_LOG_EN)
#define LOG_TIMER(format, ...) LOG_PRINT(LEVEL_DEBUG, "[TIMER]" format, ##__VA_ARGS__)
#define LOG_TIMER_ERROR(format, ...) \
    LOG_PRINT(LEVEL_DEBUG, LOG_COLOR_E "[TIMER]" format LOG_RESET_COLOR, ##__VA_ARGS__)
#define LOG_TIMER_HIGHLIGHT(format, ...) \
    LOG_PRINT(LEVEL_DEBUG, LOG_PROMPT_DISPLAY "[TIMER]" format LOG_RESET_COLOR, ##__VA_ARGS__)
#else
#define LOG_TIMER(format, ...)
#define LOG_TIMER_ERROR(format, ...)
#define LOG_TIMER_HIGHLIGHT(format, ...)
#endif

static app_timer_t* short_timer_list[SHORT_TIMER_LIST_MAX];
static app_timer_t* long_timer_list[LONG_TIMER_LIST_MAX];

static volatile uint8_t short_timer_count = 0; //短定时器数目
static volatile uint8_t long_timer_count  = 0; //长定时器数目

static volatile uint32_t millisecond_timer_count = 0; //毫秒计时器 最大 1193h
static volatile uint32_t second_timer_count      = 0; //秒计时器 最大 1193000h

static volatile uint32_t millisecond_timer_monitor_count = 0; //用来监控短定时器
static volatile uint32_t second_timer_monitor_count      = 0; //用来监控长定时器

static void timer_cleanup(app_timer_t* ptimer);
static bool timer_link(app_timer_t* ptimer);
static bool timer_unlink(app_timer_t* ptimer);
static uint8_t get_active_short_timer_count(void);

/******************************************************************************
 * @brief timer_init
 *****************************************************************************/
void timer_init(void) {
    uint8_t i;
    for (i = 0; i < SHORT_TIMER_LIST_MAX; i++) {
        short_timer_list[i] = NULL;
    }
    short_timer_count = 0;

    for (i = 0; i < LONG_TIMER_LIST_MAX; i++) {
        long_timer_list[i] = NULL;
    }
    long_timer_count = 0;
}

/******************************************************************************
 * @brief short_timer_task
 *****************************************************************************/
void short_timer_task(void) {
    app_timer_t* ptmr = NULL;
    uint8_t i;

    millisecond_timer_count++;
    for (i = 0; i < SHORT_TIMER_LIST_MAX; i++) {
        if (short_timer_list[i] != NULL) {
            ptmr = short_timer_list[i];
            // LOG_TIMER("timer_task short_timer_count%d %d %d %d 0x%x
            // ",i,short_timer_count,millisecond_timer_count,ptmr->expire,ptmr);
            if (millisecond_timer_count == ptmr->expire) {
                if (ptmr->opt >= OPT_TMR_PERIODIC) {
                    ptmr->expire = millisecond_timer_count + ptmr->period;
                } else {
                    ptmr->state = TMR_STATE_COMPLETED;
                    timer_unlink(short_timer_list[i]);
                }

                if (ptmr->callback != NULL) {
                    ptmr->callback(ptmr->context);
                }
            }
        }
    }

    //只要没有活跃的short timer 就可以关闭OS 的基准timer
    if (0 == get_active_short_timer_count()) {
        LOG_TIMER_HIGHLIGHT("NO active short timer runing -> OS base timer stop");
//        app_timer_one_millisecond_base_stop();
    }
}

/******************************************************************************
 * @brief long_timer_task
 *****************************************************************************/
void long_timer_task(void) {
    app_timer_t* ptmr = NULL;
    uint8_t i;

    second_timer_count++;

    for (i = 0; i < LONG_TIMER_LIST_MAX; i++) {
        if (long_timer_list[i] != NULL) {
            ptmr = long_timer_list[i];
            // LOG_TIMER("timer_task long_timer_count %d %d %d
            // %d",i,long_timer_count,second_timer_count,ptmr->expire);
            if (second_timer_count >= ptmr->expire) {
                if (ptmr->opt == OPT_TMR_PERIODIC) {
                    ptmr->expire = second_timer_count + ptmr->period;
                } else {
                    ptmr->state = TMR_STATE_COMPLETED;
                    timer_unlink(long_timer_list[i]);
                }

                if (ptmr->callback != NULL) {
                    ptmr->callback(ptmr->context);
                }
            }
        }
    }
}

/******************************************************************************
 * @brief timer_create
 *****************************************************************************/
bool timer_create(app_timer_t* ptimer, uint32_t period, uint16_t opt, tmr_fnct_ptr pcallback) {
    if (ptimer == NULL)
        return false;

    ptimer->expire   = 0;
    ptimer->period   = period;
    ptimer->opt      = opt;
    ptimer->callback = pcallback;
    ptimer->state    = TMR_STATE_STOPPED;
    return true;
}

/******************************************************************************
 * @brief timer_del
 *****************************************************************************/
bool timer_del(app_timer_t* ptimer) {
    if (!ptimer)
        return false;
    timer_cleanup(ptimer);
    ptimer = NULL;
    return true;
}

/******************************************************************************
 * @brief timer_start
 *****************************************************************************/
bool timer_start(app_timer_t* ptimer) {
    if (!ptimer)
        return false;

    switch (ptimer->state) {
        case TMR_STATE_RUNNING:
            timer_unlink(ptimer); // 注意:这里没有break是故意的，继续执行后面的case操作
        case TMR_STATE_STOPPED:
        case TMR_STATE_COMPLETED:
            timer_link(ptimer);
            break;

        default:
            return false;
    }

#if (1 == DEBUG_TIMER)
    LOG_TIMER("start runing timer id %d", ptimer->timer_id);
#endif

#if 0 //TODO
    if ((false == ptimer->is_long_timer) &&
        (OS_E_SWTIMER_RUNNING != get_app_timer_one_millisecond_base_status())) {
        LOG_TIMER_HIGHLIGHT("FIRST short timer start -> OS base timer start");
        app_timer_one_millisecond_base_start();
    }
#endif
    return true;
}

/******************************************************************************
 * @brief timer_stop
 *****************************************************************************/
bool timer_stop(app_timer_t* ptimer) {
    if (!ptimer)
        return false;

    return timer_unlink(ptimer);
}

/******************************************************************************
 * @brief check_timer_runing
 *****************************************************************************/
bool timer_runing_check(app_timer_t* ptimer) {
    uint8_t i;
    bool result = false;
    if (ptimer->is_long_timer) {
        for (i = 0; i < LONG_TIMER_LIST_MAX; i++) {
            if (long_timer_list[i] == ptimer) {
                ((ptimer->state == TMR_STATE_RUNNING) ? (result = true) : (result = false));
            }
        }

    } else {
        for (i = 0; i < SHORT_TIMER_LIST_MAX; i++) {
            if (short_timer_list[i] == ptimer) {
                ((ptimer->state == TMR_STATE_RUNNING) ? (result = true) : (result = false));
            }
        }
    }
    return result;
}

/******************************************************************************
 * @brief timer_cleanup
 *****************************************************************************/
static void timer_cleanup(app_timer_t* ptimer) {
    if (!ptimer)
        return;
    ptimer->state    = TMR_STATE_UNUSED;
    ptimer->callback = NULL;
    ptimer->expire   = 0;
    ptimer->opt      = 0;
}

/******************************************************************************
 * @brief timer_link
 *****************************************************************************/
static bool timer_link(app_timer_t* ptimer) {
    uint8_t i;

    if (!ptimer)
        return false;

    if (short_timer_count >= SHORT_TIMER_LIST_MAX)
        return false;

    if (long_timer_count >= LONG_TIMER_LIST_MAX)
        return false;

    if (ptimer->is_long_timer) {
        for (i = 0; i < LONG_TIMER_LIST_MAX; i++) {
            if (long_timer_list[i] == NULL) {
                ptimer->state      = TMR_STATE_RUNNING;
                ptimer->expire     = ptimer->period + second_timer_count;
                long_timer_list[i] = ptimer;
                long_timer_count++;
                return true;
            }
        }

    } else {
        for (i = 0; i < SHORT_TIMER_LIST_MAX; i++) {
            if (short_timer_list[i] == NULL) {
                ptimer->state       = TMR_STATE_RUNNING;
                ptimer->expire      = ptimer->period + millisecond_timer_count;
                short_timer_list[i] = ptimer;
                short_timer_count++;
                return true;
            }
        }
    }

    return false;
}

/******************************************************************************
 * @brief timer_unlink
 *****************************************************************************/
static bool timer_unlink(app_timer_t* ptimer) {
    uint8_t i;

    if (!ptimer)
        return false;

    if (ptimer->is_long_timer) {
        for (i = 0; i < LONG_TIMER_LIST_MAX; i++) {
            if (long_timer_list[i] == ptimer) {
                long_timer_list[i] = NULL;
                ptimer->state      = TMR_STATE_STOPPED;
                long_timer_count--;
                break;
            }
        }
    } else {
        for (i = 0; i < SHORT_TIMER_LIST_MAX; i++) {
            if (short_timer_list[i] == ptimer) {
                short_timer_list[i] = NULL;
                ptimer->state       = TMR_STATE_STOPPED;
                short_timer_count--;
                break;
            }
        }
    }

    return true;
}

/******************************************************************************
 * @brief get_active_short_timer_count
 *****************************************************************************/
static uint8_t get_active_short_timer_count(void) {
    uint8_t i                 = 0;
    uint8_t short_timer_count = 0;
    for (i = 0; i < SHORT_TIMER_LIST_MAX; i++) {
        if (TMR_STATE_RUNNING == short_timer_list[i]->state) {
            short_timer_count++;
        }
    }
    return short_timer_count;
}

/******************************************************************************
 * @brief force_stop_all_short_timer
 *****************************************************************************/
void force_stop_all_short_timer(void) {
    LOG_TIMER("force stop all short timer !!!");
    //强制检查 所有runing 状态 注册的 callback 函数并且执行一次
    app_timer_t* ptmr = NULL;
    uint8_t i;
    for (i = 0; i < SHORT_TIMER_LIST_MAX; i++) {
        if (short_timer_list[i] != NULL) {
            ptmr = short_timer_list[i];

            if ((ptmr->state == TMR_STATE_RUNNING) && (ptmr->callback != NULL)) {
                ptmr->state = TMR_STATE_STOPPED;
                ptmr->callback(ptmr->context);
            }
        }
    }

//    app_timer_one_millisecond_base_stop();
}

/******************************************************************************
 * @brief force_stop_all_long_timer
 *****************************************************************************/
void force_stop_all_long_timer(void) {

}

/******************************************************************************
 * @brief app_timer_self_check_hander
 *****************************************************************************/
app_timer_error_code_type app_timer_self_check_hander(void) {

    return APP_TIMER_OK;
}
