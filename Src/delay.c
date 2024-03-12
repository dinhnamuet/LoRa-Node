#include "delay.h"

void Timer_Config()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 72 - 1; 
    TIM3->ARR = 0xFFFF; 
		TIM3->EGR |= TIM_EGR_UG;
    TIM3->CR1 |= TIM_CR1_CEN; 
}
void delay_init(void)
{
    Timer_Config(); 
    while (!(TIM3->SR & TIM_SR_UIF)) {}
    TIM3->SR &= ~TIM_SR_UIF;
}
void delay_us(uint32_t us)
{
    while (us) 
    {
        TIM3->CNT = 0; 
        while (TIM3->CNT < 1) {} 
        us--;
    }
}
void delay_ms(uint32_t msgiay)
{
	while (msgiay--)
	{
		delay_us(1000);
	}
}