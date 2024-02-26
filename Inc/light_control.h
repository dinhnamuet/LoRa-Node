#ifndef __LIGHT_CONTROL_H__
#define __LIGHT_CONTROL_H__
#include <stdint.h>
#include "mesh.h"
uint8_t getDutyCycle_Manual(struct time_set foo);
uint8_t getDutyCycle_Auto(uint32_t light_sensor);
#endif
