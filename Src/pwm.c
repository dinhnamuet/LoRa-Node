#include "pwm.h"
#include "stm32f10x.h"
void pwm_init(void)
{
	RCC->APB1ENR |= 0x01;
	RCC->APB2ENR |= (1UL<<2);
	GPIOA->CRL |= (1UL<<0)|(1UL<<1)|(1UL<<3);
	GPIOA->CRL &= ~(1UL<<2);
	
	TIM2->CR1 &= ~(1UL<<4); //up counter
	TIM2->CCER |= 0x01;
	TIM2->CCER &= ~(1UL<<1);
	TIM2->CCMR1 |= (0x06<<4) | (1<<3);
	TIM2->CNT = 0;
	TIM2->PSC = 0;
	TIM2->EGR |= TIM_EGR_UG;
}
void pwm_setFrequency(uint32_t freq)
{
	TIM2->ARR = (uint16_t)(SystemCoreClock/freq);
}
void pwm_setDutyCycle(uint8_t dc)
{
	if(dc <= 100)
		TIM2->CCR1 = (dc*TIM2->ARR)/100;
}
void pwm_ctl(uint8_t ctl)
{
	if(ctl)
		TIM2->CR1 |= 0x01;
	else
		TIM2->CR1 &= ~0x01;
}
