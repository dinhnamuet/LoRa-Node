#ifndef __MOTION_H__
#define __MOTION_H__
#include <stdint.h>

void motion_sensor_PC13_init(void);
uint8_t get_motion_sensor_value(void);

#endif
