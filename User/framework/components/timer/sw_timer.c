#include "sw_timer.h"
#include "drv_timer.h"
#include "log.h"

/* 软定时器抽象层结构体，其中timer变量类型根据平台适配更改 */
typedef struct {
    bool is_used;      // 是否已经初始化
    app_timer_t timer; // app_timer 结构
    sw_timer_cb cb;    // 指向实际的回调函数
    void* context;     // 回调函数携带的参数
} sw_timer_t;

/* 全局的软定时器列表 */
static sw_timer_t m_sw_timer_list[MAX_SW_TIMER_ID_COUNT];

/* 软定时器抽象模块初始化 */
void sw_timer_init(void) {
    memset(m_sw_timer_list, 0, sizeof(m_sw_timer_list));
    timer_init();

#if 0
    uint8_t i;
    for( i = 0; i <= SW_TIMER_ID_MAX; i++)
    {
        LOG("TIMER ADDR 0x%x",&m_sw_timer_list[i].timer);
    }
#endif
}

/* 所有的平台定时器超时回调函数都使用同一个接口，根据user参数所指向的具体sw_timer来调用实际的回调函数
 */
static void sw_timer_timeout_handler(void* user) {
    if (NULL == user)
        return;

    // 取m_sw_timer_list里的元素
    sw_timer_t* user_timer = (sw_timer_t*)user;
    // 调用实际的定义在m_sw_timer_list里的回调
    if (user_timer->is_used && user_timer->cb)
        user_timer->cb(user_timer->context);
}

/* 创建软件定时器 */
void sw_timer_create(sw_timer_id id, sw_timer_type type, sw_timer_cb cb) {
    if (id >= MAX_SW_TIMER_ID_COUNT)
        return;

    if (m_sw_timer_list[id].is_used)
        return;

    if (SW_TIMER_TYPE_SINGLE != type && SW_TIMER_TYPE_REPEATED != type)
        return;

    // 初始化其他变量
    m_sw_timer_list[id].is_used = 1;
    m_sw_timer_list[id].cb      = cb;
    m_sw_timer_list[id].context = NULL;

    //根据ID 判断是为长定时
    if (id < LONG_SW_TIMER_ID_START_INDEX) {
        m_sw_timer_list[id].timer.is_long_timer = false; // 设置为短定时器
#if (1 == DEBUG_TIMER)
        m_sw_timer_list[id].timer.timer_id = id;
#endif
    } else {
        m_sw_timer_list[id].timer.is_long_timer = true; // 设置为长定时器
#if (1 == DEBUG_TIMER)
        m_sw_timer_list[id].timer.timer_id = id;
#endif
    }

    /*
     * 调用平台软定时器实际创建接口，注意:
     * 1.这里平台接口参数应该是使用m_sw_timer_list对应元素的变量
     * 2.这里的回调函数统一使用一个接口，在接口中再执行具体的回调s
     */
    timer_create(&m_sw_timer_list[id].timer, 0,
                 (SW_TIMER_TYPE_REPEATED == type) ? OPT_TMR_PERIODIC : OPT_TMR_ONE_SHOT,
                 sw_timer_timeout_handler);
}

/* 启动软件定时器 */
void sw_timer_start(sw_timer_id id, uint32_t timeout_value, sw_timer_cb cb, void* context) {
    if (id >= MAX_SW_TIMER_ID_COUNT)
        return;

    if (0 == m_sw_timer_list[id].is_used)
        return;

    // 这里如果平台软件定时器允许传入新的回调函数，则进行更新，一般传NULL
    if (cb)
        m_sw_timer_list[id].cb = cb;

    if (context)
        m_sw_timer_list[id].context = context;
    else
        m_sw_timer_list[id].context = NULL;

    //根据ID 判断石为长定时
    if (m_sw_timer_list[id].timer.is_long_timer) // 启动长定时器
    {
        if (0 != timeout_value)
            m_sw_timer_list[id].timer.period = SW_LONG_APP_TIME_TICK(timeout_value);
    } else // 启动短定时器
    {
        if (0 != timeout_value)
            m_sw_timer_list[id].timer.period = SW_SHORT_APP_TIME_TICK(timeout_value);
    }

    m_sw_timer_list[id].timer.context = (void*)&m_sw_timer_list[id];

#if (1 == DEBUG_TIMER)
    m_sw_timer_list[id].timer.timer_id = id;
#endif
    /*
     * 调用平台软定时器实际start接口，注意:
     * 1.这里如果平台接口没有先stop再restart，需要手动先调用stop接口停止定时器
     * 2.这里的参数根据平台接口选择m_sw_timer_list对应元素里的变量
     * 3.timeout_value如果平台接口层没有自动进行转换到ms数量，需要先转换
     */
    timer_start(&m_sw_timer_list[id].timer);
}

/* 停止软件定时器 */
void sw_timer_stop(sw_timer_id id) {
    if (id >= MAX_SW_TIMER_ID_COUNT)
        return;

    if (0 == m_sw_timer_list[id].is_used)
        return;

    // 调用平台软定时器实际stop接口
    timer_stop(&m_sw_timer_list[id].timer);
}

/* 检查当前软定时器是否正在运行，true:正在运行;false:没有运行 */
bool sw_timer_check_running(sw_timer_id id) {
    if (id >= MAX_SW_TIMER_ID_COUNT)
        return false;

    if (0 == m_sw_timer_list[id].is_used)
        return false;

    // 调用平台软定时器实际stop接口
    return timer_runing_check(&m_sw_timer_list[id].timer);
}

bool sw_timer_abnormal_check(void) {
    if (APP_TIMER_ERROR == app_timer_self_check_hander()) {
        return true;
    } else {
        return false;
    }
}

void sw_timer_stop_all_force(void) {
    force_stop_all_short_timer();
    force_stop_all_long_timer();
}

static void aging_task_timer_handler(void* user);
static uint8_t data = 0x55;
void sw_timer_test(void) {
    SW_TIMER_CREATE(SW_TIMER_ID_TEST, SW_TIMER_TYPE_REPEATED, aging_task_timer_handler);
    SW_TIMER_START_WITH_DATA(SW_TIMER_ID_TEST, 5000, &data);
}

static void aging_task_timer_handler(void* user) {
    LOG_HIGHLIGHT("aging_task_timer_handler 0x%x", (*((uint8_t*)user)));
    SW_TIMER_STOP(SW_TIMER_ID_TEST);
}
