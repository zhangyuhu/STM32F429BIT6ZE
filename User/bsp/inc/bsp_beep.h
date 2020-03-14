#ifndef __BSP_BEEP_H
#define __BSP_BEEP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define BEEP_INIT()                        beep_init()
#define BEEP_SHUTDOWN()                    beep_shutdown()
#define CHECK_BEEP_DRIVING_FLAG()          check_beep_driving_flag()
#define BEEP_START(level, beep_array)      beep_start(level, beep_array)
#define GET_BEEP_VOLUNE_LEVEL()            get_beep_volume_level()
#define SET_BEEP_VOLUNE_LEVEL(level)       set_beep_volume_level(level)
#define BEEP_START_BASS(level, beep_array) beep_start_bass(level, beep_array)

#define BEEP_LEVEL_ALARM    (1)
#define BEEP_LEVEL_REGISTER (5)
#define BEEP_LEVEL_NORMAL   (10)
#define BEEP_LEVEL_NONE     (99)

extern const int BEEP_RAISE[];
extern const int BEEP_RAISE_LOW[];
extern const int BEEP_REDUCE[];
extern const int BEEP_RAISE_ALL[];
extern const int BEEP_REDUCE_ALL[];

extern const int BEEP_DI[];
extern const int BEEP_DIDI[];
extern const int DELAY_AND_BEEP_DIDI[];
extern const int BEEP_FORBID[];
extern const int BEEP_STOP[];

extern const int BEEP_2S[];
extern const int BEEP_ALARM_5S[];

typedef enum {
    nomal_level  = 0x01,
    bass_level   = 0x02,
    silent_level = 0x03,
} volume_level_t;

/* Public functions declaration */

int beep_init(void);
int beep_shutdown(void);
bool check_beep_driving_flag(void);
int beep_start(int level, const int beep_array[]);

volume_level_t get_beep_volume_level(void);
void set_beep_volume_level(volume_level_t level);
void beep_start_bass(int level, const int beep_array[]);

void beep_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __BEEP_C */

