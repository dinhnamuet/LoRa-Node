#ifndef __ADC_NAM_H__
#define __ADC_NAM_H__
#include <stdint.h>
void adc_init(void);
uint16_t read_adc(uint8_t channel);
#endif
