
#ifndef __LOG_H_
#define __LOG_H_

#if defined __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bsp.h"

#define DEBUG_LOG 			(1)
#define LOG_TIME_PRINT 		(0)


#if 1
#define DBG_vInit()     do{log_init();}while(0)
#define DBG_Disable() 	do{}while(0)
#else
#define DBG_vInit()			do{}while(0)
#define DBG_Disable()		do{}while(0)
#endif




// level
#define LEVEL_CLOSE        (1)
#define LEVEL_SIMPLE_FORCE (4)
#define LEVEL_FORCE        (5)

#define LEVEL_CLI     (9)
#define LEVEL_RELEASE (10)
#define LEVEL_SIMPLE  (11)
#define LEVEL_DEBUG   (12)
#define LEVEL_INFO    (13)
#define LEVEL_WARNING (14)
#define LEVEL_ERROR   (15)
#define __LEVEL__     (LEVEL_ERROR)

//日志颜色配置
#define LOG_COLOR_CONFIG_EN (1 == DEBUG_LOG)

//日志颜色
#if (1 == LOG_COLOR_CONFIG_EN)
#define LOG_COLOR_BLACK  "30"
#define LOG_COLOR_RED    "31"
#define LOG_COLOR_GREEN  "32"
#define LOG_COLOR_BROWN  "33"
#define LOG_COLOR_BLUE   "34"
#define LOG_COLOR_PURPLE "35"
#define LOG_COLOR_CYAN   "36"
#define LOG_COLOR_WHITE  "37"

#define LOG_BACKGROUND_BLACK  "40"
#define LOG_BACKGROUND_RED    "41"
#define LOG_BACKGROUND_GREEN  "42"
#define LOG_BACKGROUND_BROWN  "43"
#define LOG_BACKGROUND_BLUE   "44"
#define LOG_BACKGROUND_PURPLE "45"
#define LOG_BACKGROUND_CYAN   "46"
#define LOG_BACKGROUND_WHITE  "47"

#define LOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)  "\033[1;" COLOR "m"
#define LOG_RESET_COLOR  "\033[0m"
#define LOG_COLOR_E      LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W      LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I      LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D
#define LOG_COLOR_V

#define LOG_BOLD_E LOG_BOLD(LOG_BACKGROUND_RED)
#define LOG_BOLD_W LOG_BOLD(LOG_BACKGROUND_BROWN)
#define LOG_BOLD_I LOG_BOLD(LOG_BACKGROUND_GREEN)
#define LOG_BOLD_D
#define LOG_BOLD_V

#define LOG_DISPLAY(COLOR, BACKGROUND) "\033[1;" COLOR ";" BACKGROUND "m"
#define LOG_PROMPT_DISPLAY             LOG_DISPLAY(LOG_COLOR_BROWN, LOG_BACKGROUND_BLACK)


#else

#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#define LOG_RESET_COLOR

#define LOG_BOLD_E
#define LOG_BOLD_W
#define LOG_BOLD_I
#define LOG_BOLD_D
#define LOG_BOLD_V

#define LOG_PROMPT_DISPLAY
#endif

void App_Printf(char *format, ...);


#if (DEBUG_LOG == 1)
#define LOG(format,...) {App_Printf(format, ##__VA_ARGS__);App_Printf(LOG_STRING_NEW_LINE);}while(0)
#define LOG_SIMPLE(format, ...) {App_Printf(format, ##__VA_ARGS__);  }while(0)
#define LOG_ERROR(format, ...)   {App_Printf(LOG_COLOR_E format LOG_RESET_COLOR, ##__VA_ARGS__); App_Printf(LOG_STRING_NEW_LINE);  }while(0)
#define LOG_FORCE(format, ...) {App_Printf(LOG_COLOR_E format LOG_RESET_COLOR, ##__VA_ARGS__); App_Printf(LOG_STRING_NEW_LINE);  }while(0)
#define LOG_HIGHLIGHT(format, ...) {App_Printf(LOG_PROMPT_DISPLAY format LOG_RESET_COLOR, ##__VA_ARGS__); App_Printf(LOG_STRING_NEW_LINE);  }while(0)
#else
#define LOG(format, ...)
#define LOG_SIMPLE(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_FORCE(format, ...)
#define LOG_HIGHLIGHT(format, ...)
#endif

#define LOG_STRING_NEW_LINE "\r\n"

#if defined __cplusplus
}
#endif

#endif
