#ifndef _SW_TIMER_H
#define _SW_TIMER_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "drv_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OS_APP_TIME_MS(t) (16000UL * (t))
#define SW_SHORT_APP_TIME_TICK(t)                                                   \
    ((t >= SW_TIMER_BASE_CLOCK_INTERVAL_MS) ? (t / SW_TIMER_BASE_CLOCK_INTERVAL_MS) \
                                            : ((t / SW_TIMER_BASE_CLOCK_INTERVAL_MS) + 1))
#define SW_LONG_APP_TIME_TICK(t)                      \
    ((t >= SW_TIMER_BASE_CLOCK_LONG_INTERVAL_MS)      \
         ? (t / SW_TIMER_BASE_CLOCK_LONG_INTERVAL_MS) \
         : ((t / SW_TIMER_BASE_CLOCK_LONG_INTERVAL_MS) + 1))

/* 定义当前用到的所有软定时器的所有ID
 * 如果有增加或者删除，需要更改这里的值
 */
typedef enum {

	  SW_TIMER_ID_TEST,
    // 以下timer间隔较长，需要使用长定时器
    // 功耗调优版本，长定时时长最好为5s的整数倍
    SW_TIMER_ID_LONG_TIMER_INDEX,



    // 最后一个TIMER，但是不能使用
    SW_TIMER_ID_MAX,
} sw_timer_id;

typedef enum {
    SW_TIMER_TYPE_SINGLE,   // 每次启动只执行一次
    SW_TIMER_TYPE_REPEATED, // 超时后继续自动启动
} sw_timer_type;

/* 软定时器最大数量，取sw_timer_id枚举类型的最大值 */
#define MAX_SW_TIMER_ID_COUNT        (SW_TIMER_ID_MAX)
#define LONG_SW_TIMER_ID_START_INDEX (SW_TIMER_ID_LONG_TIMER_INDEX)

typedef void (*sw_timer_cb)(void* p_context);

/* 其他模块都统一使用宏函数调用 */
#define SW_TIMER_CREATE(id, type, cb)               sw_timer_create(id, type, cb)
#define SW_TIMER_START(id, timeout)                 sw_timer_start(id, timeout, NULL, NULL)
#define SW_TIMER_START_WITH_CB(id, timeout, cb)     sw_timer_start(id, timeout, cb, NULL)
#define SW_TIMER_START_WITH_DATA(id, timeout, data) sw_timer_start(id, timeout, NULL, data)
#define SW_TIMER_STOP(id)                           sw_timer_stop(id)
#define SW_TIMER_CHECK_RUNNING(id)                  sw_timer_check_running(id)
#define SW_TIMER_FORCE_STOP_ALL()                   sw_timer_stop_all_force()
#define SW_TIMER_ABNORMAL_CHECK()                   sw_timer_abnormal_check()

/* 对外接口函数 */
void sw_timer_init(void);
void sw_timer_create(sw_timer_id id, sw_timer_type type, sw_timer_cb cb);
void sw_timer_start(sw_timer_id id, uint32_t timeout_value, sw_timer_cb cb, void* context);
void sw_timer_stop(sw_timer_id id);
bool sw_timer_check_running(sw_timer_id id);
bool sw_timer_abnormal_check(void);
void sw_timer_stop_all_force(void);

void sw_timer_test(void);

#ifdef __cplusplus
}
#endif

#endif /* _SW_TIMER_H */
