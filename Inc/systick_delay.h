#ifndef __SYSTICK_DELAY_H__
#define __SYSTICK_DELAY_H__
#include <stdint.h>
#include "stm32f10x.h"
void delay_us(uint32_t us_t);
void delay_ms(uint32_t ms_t);
#endif
