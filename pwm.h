#ifndef __PWM_H__
#define __PWM_H__
#include <stdint.h>
#define START 1
#define STOP 0
void pwm_init(void);
void pwm_setFrequency(uint32_t freq);
void pwm_setDutyCycle(uint8_t dc);
void pwm_ctl(uint8_t ctl);
#endif
