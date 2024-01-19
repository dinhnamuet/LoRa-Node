#include "adc_nam.h"
#include "stm32f10x.h"
#include "systick_delay.h"
#define set_bit(port, bit) port|=(1UL<<bit)
#define reset_bit(port, bit) port&=~(1UL<<bit)
static void start_adc_conversion(void)
{
	set_bit(ADC1->CR2, 0);
	delay_ms(10);
	set_bit(ADC1->CR2, 0);
	delay_ms(10);
	set_bit(ADC1->CR2, 22);
}
static void location(uint8_t channel)
{
	uint32_t mid;
	mid = ADC1->SQR3>>(5*channel);
	mid &= 0x1f;
	ADC1->SQR3 |= mid;
	ADC1->SQR3 &= ~(unsigned int)(0x1F<<(5*channel));
}
static void relocation(uint8_t channel)
{
	uint32_t mid;
	mid = ADC1->SQR3 & 0x1F;
	ADC1->SQR3 |= (mid << (5*channel));
	ADC1->SQR3 &= ~(unsigned int)(0x1F);
}
static void set_channel(uint8_t channel)
{
	reset_bit(GPIOA->CRL, (4*channel));
	reset_bit(GPIOA->CRL, (4*channel + 1));
	reset_bit(GPIOA->CRL, (4*channel + 2));
	reset_bit(GPIOA->CRL, (4*channel + 3));
}
void adc_init(void)
{
	set_bit(RCC->APB2ENR, 9); //enable clock ADC1 peripheral
	set_bit(RCC->APB2ENR, 2);
	set_bit(ADC1->CR1, 11);
	reset_bit(ADC1->CR2, 11);
	reset_bit(ADC1->CR2, 1);
	ADC1->CR1 |= (0x07 << 13);
	ADC1->CR2 |= (0x07 << 17);
	ADC1->SQR1 =(unsigned int)(15UL<<20);
	ADC1->SQR3 = (1UL<<5) | (2UL<<10) | (3UL<<15) | (4UL<<20) | (5UL<<25);
	ADC1->SQR2 = (6UL<<0) | (7UL<<5) | (8UL<<10) | (9UL<<15) | (10UL<<20) | (11UL<<25);
	ADC1->SQR1 = (12UL<<0) | (13UL<<5) | (14UL<<10) | (15UL<<15);
}
uint16_t read_adc(uint8_t channel)
{
	set_channel(channel);
	ADC1->SMPR2 |= (unsigned int)(0x07<<(3*channel));
	location(channel);
	start_adc_conversion();
	while(!(ADC1->SR & (1UL<<1)));
	relocation(channel);
	return (ADC1->DR & 0xFFF);
}
