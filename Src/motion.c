#include "motion.h"
#include "stm32f10x.h"

void motion_sensor_PC13_init(void)
{
	RCC->APB2ENR |= (1UL<<4);
	GPIOC->CRH |= (1UL<<22);
	GPIOC->CRH &= ~( (1UL<<20) | (1UL<<21) | (1UL<<23) );
	GPIOC->ODR &= ~(1UL<<13);
}
uint8_t get_motion_sensor_value(void)
{
	return ((GPIOC->IDR)>>13)&0x01;
}
