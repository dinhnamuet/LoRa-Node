/*
	DHT attach PC14
*/
#include <stdlib.h>
#include "dht11_nam.h"
#include "systick_delay.h"
#include "stm32f10x.h"
#define set(port,bit) port|=(1U<<bit)
#define reset(port,bit) port&=~(1U<<bit)
#define toggle(port,bit) port^=(1U<<bit)
extern volatile int counting;
static void set_counter_value(int value)
{
	counting = value;
}
static void counter_enable(void)
{
	SysTick->CTRL |= (1U<<0);
	SysTick->CTRL |= (1UL<<1);
}
static void counter_disable(void)
{
	SysTick->CTRL &= ~(1U<<0);
	SysTick->CTRL &= ~(1UL<<1);
}
static int get_counter_value(void)
{
	return counting;
}
void dht11_init(void)
{
	set(RCC->APB2ENR, 4);
	NVIC_EnableIRQ(SysTick_IRQn);
	counter_disable();
	counting = 0;
}

static void set_direction(uint8_t dir)
{
	if(dir == OUTPUT)
	{
		set(GPIOC->CRH, 24);
		set(GPIOC->CRH, 25);
		set(GPIOC->CRH, 26);
		reset(GPIOC->CRH, 27);
	}
	else if(dir == INPUT)
	{
		reset(GPIOC->CRH, 24);
		reset(GPIOC->CRH, 25);
		set(GPIOC->CRH, 26);
		reset(GPIOC->CRH, 27);
	}
}
static uint8_t read_pin_value(void)
{
	return ((GPIOC->IDR)>>14)&0x01;
}
static void start_condition(void)
{
	set_direction(OUTPUT);
	reset(GPIOC->ODR, 14);
	delay_ms(19);
	set(GPIOC->ODR, 14);
}
static int8_t check_response(void)
{
	set_direction(INPUT);
	while(1)
	{
		if(read_pin_value())
			break;
	}
	while(read_pin_value());
	set_counter_value(0);
	counter_enable();
	while(!read_pin_value())
	{
		if(get_counter_value() >= 90)
		{
			counter_disable();
			return -1;
		}
	}
	if(get_counter_value() < 60)
	{
		counter_disable();
		return -1;
	}
	counter_disable();
	return 0;
}
int8_t read_data(double *tem, double *hum)
{
	int8_t i;
	double tp;
	static uint8_t first_tem	= 0;
	static uint8_t last_tem		= 0;
	static uint8_t first_hum	= 0;
	static uint8_t last_hum		= 0;
	static uint8_t check_sum	= 0;
	start_condition();
	check_response();
	if(1)
	{
		while(read_pin_value() == 1);
		for(i = 7; i>=0; i--)
		{
			while(read_pin_value() == 0);
			set_counter_value(0);
			counter_enable();
			while(read_pin_value() == 1);
			if(get_counter_value() < 40)
				reset(first_hum, i);
			else
				set(first_hum, i);
			counter_disable();
		}
		for(i = 7; i>=0; i--)
		{
			while(read_pin_value() == 0);
			set_counter_value(0);
			counter_enable();
			while(read_pin_value() == 1);
			if(get_counter_value() < 40)
				reset(last_hum, i);
			else
				set(last_hum, i);
			counter_disable();
		}
		for(i = 7; i>=0; i--)
		{
			while(read_pin_value() == 0);
			set_counter_value(0);
			counter_enable();
			while(read_pin_value() == 1);
			if(get_counter_value() < 40)
				reset(first_tem, i);
			else
				set(first_tem, i);
			counter_disable();
		}
		for(i = 7; i>=0; i--)
		{
			while(read_pin_value() == 0);
			set_counter_value(0);
			counter_enable();
			while(read_pin_value() == 1);
			if(get_counter_value() < 40)
				reset(last_tem, i);
			else
				set(last_tem, i);
			counter_disable();
		}
		for(i = 7; i>=0; i--)
		{
			while(read_pin_value() == 0);
			set_counter_value(0);
			counter_enable();
			while(read_pin_value() == 1);
			if(get_counter_value() < 40)
				reset(check_sum, i);
			else
				set(check_sum, i);
			counter_disable();
		}
		tp = (last_tem<99)? (double)last_tem/100 : (double)last_tem/1000;
		*tem = (double)(first_tem) + tp;
		tp = (last_hum<99)? (double)last_hum/100 : (double)last_hum/1000;
		*hum = (double)(first_hum) + tp;
		if(first_tem + last_tem + first_hum + last_hum == check_sum)
			return 0;
		else
			return -1;
	}
}
double readTemperature(void)
{
	double temperature;
	read_data(&temperature, NULL);
	return temperature;
}
double readHumidity(void)
{
	double humidity;
	read_data(NULL, &humidity);
	return humidity;
}
