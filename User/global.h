#ifndef __GLOBAL_H
#define __GLOBAL_H

// c 库文件
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// flash layout
//#include "flash_layout.h"

//配置文件
#include "bsp.h"

//#include "bsp_config.h"
//#include "module_config.h"


// log
#include "log.h"

/*****************************************
 *版本
 *****************************************/

#define VERSION(a, b, c, d) ((uint32_t)((a << 24) + (b << 16) + (c << 8) + d))

#define APP_BASE_VERSION VERSION(0, 0, 0, 0)


#define APP_VERSION (APP_BASE_VERSION)




#endif /* __GLOBAL_H */
