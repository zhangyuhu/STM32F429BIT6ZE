#ifndef __DRV_TIMER_H
#define __DRV_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#ifndef RELEASE_VERSION_ENABLE
#define DEBUG_TIMER (0)
#else
#define DEBUG_TIMER (0)
#endif

//软件定时器的基础时钟心跳
#define SW_TIMER_BASE_CLOCK_INTERVAL_MS      (10)
#define SW_TIMER_BASE_CLOCK_LONG_INTERVAL_MS (5000)

#define SHORT_TIMER_LIST_MAX (20) //目前使用了18个
#define LONG_TIMER_LIST_MAX  (15) //目前使用了14个

typedef void (*tmr_fnct_ptr)(void* p_context);

typedef struct {
    bool is_long_timer;
    uint32_t expire;
    uint32_t period;
    tmr_fnct_ptr callback;
    uint8_t state;
    uint32_t opt;
    void* context;
#if (1 == DEBUG_TIMER)
    uint8_t timer_id;
#endif
} app_timer_t;

/* state */
#define TMR_STATE_UNUSED    0
#define TMR_STATE_STOPPED   1
#define TMR_STATE_RUNNING   2
#define TMR_STATE_COMPLETED 3

/* option */
#define OPT_TMR_ONE_SHOT 1 // Timer will not auto restart when it expires
#define OPT_TMR_PERIODIC 2 // Timer will     auto restart when it expires

// error code

typedef enum {
    APP_TIMER_NONE  = 0,
    APP_TIMER_OK    = 1,
    APP_TIMER_ERROR = 2,
} app_timer_error_code_type;

void timer_init(void);

/*
 * brief      create a timer
 * para opt   can be OPT_TMR_ONE_SHOT or OPT_TMR_PERIODIC
 */
bool timer_create(app_timer_t* ptimer, uint32_t period, uint16_t opt, tmr_fnct_ptr pcallback);

bool timer_del(app_timer_t* ptimer);

bool timer_start(app_timer_t* ptimer);

bool timer_stop(app_timer_t* ptimer);

bool timer_runing_check(app_timer_t* ptimer);

app_timer_error_code_type app_timer_self_check_hander(void);

/*
 * brief     timer process must called one tick
 */
void short_timer_task(void);
void long_timer_task(void);

void force_stop_all_short_timer(void);
void force_stop_all_long_timer(void);

#ifdef __cplusplus
}
#endif

#endif
