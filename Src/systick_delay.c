#include "systick_delay.h"
#define ENABLE 	1
#define DISABLE 0
volatile int counting = 0;
static void delay_init(void)
{
	SysTick->CTRL 	|= (1UL<<2);
	SysTick->LOAD 	= 71;
	SysTick->VAL	= 0;
}
static void counter_ctl(uint8_t mode)
{
	if(mode)
		SysTick->CTRL |= (1UL<<0);
	else
		SysTick->CTRL &= ~(1UL<<0);
}
void delay_us(uint32_t us_t)
{
	delay_init();
	counter_ctl(ENABLE);
	while(us_t)
	{
		if(SysTick->CTRL & (1UL<<16))
		{
			SysTick->CTRL &= ~(1UL<<16);
			--us_t;
		}
	}
	counter_ctl(DISABLE);
}
void delay_ms(uint32_t ms_t)
{
	while(ms_t)
	{
		delay_us(1000);
		--ms_t;
	}
}
void SysTick_Handler(void) {
	counting += 1;
}
