/*
	implement watchdog driver 
*/
#include "watchdog.h"
#include "stm32f10x.h"
#include "systick_delay.h"
void watchdog_init(void)
{
	RCC->CSR |= 0x01;
	while(!(RCC->CSR & 0x02));
	IWDG->KR = 0x5555;
	while((IWDG->SR & (0x03)));
	IWDG->PR	= 0x06;
	IWDG->RLR = 0xFFF;
	IWDG->KR	= 0xCCCC;
}
void ping_to_wdt(void)
{
	IWDG->KR = 0xAAAA;
	delay_ms(3);
	IWDG->KR = 0xCCCC;
}
