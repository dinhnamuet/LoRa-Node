/*
	TITLE: implement lora mesh
	DESCRIPTION: light monitoring system
	AUTHOR: Dinh Huu Nam
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "systick_delay.h"
#include "lcd_hehe.h"
#include "adc_nam.h"
#include "wdt_nam.h"
#include "lora.h"
#include "mesh_nam.h"
#include "pwm.h"
const uint32_t NODE_ID = 20021165;

static volatile uint8_t rx_sig = 0;
char tx_buff[50];
struct time curTime;
struct time_set timeSetting;
struct LoRa_Setup myLoRa;
extern void SysTick_Handler(void);
static void get_data(void);
static uint8_t recv_data[50];
void EXTI2_IRQHandler(void);
struct LoRa_node myNode;
static void manual_mode(struct time_set foo, uint8_t *duty_cycle);
static void auto_mode(uint32_t light_sensor, uint8_t *duty_cycle);
int main(void)
{
	uint8_t last_dt = 0;
	char buffer[50];
	myLoRa = newLora();
	lcd_init();
	while (LoRa_init(&myLoRa) != LORA_OK)
	{
		lcd_goto_xy(0,0);
		lcd_display("Please Wait...!");
		delay_ms(100);
	}
	adc_init();
	pwm_init();
	pwm_setFrequency(100);
	pwm_setDutyCycle(0);
	pwm_ctl(START);
	clear_lcd();
	LoRa_startReceiving(&myLoRa);
	delay_ms(2000);
	watchdog_init();
	clear_lcd();
	myNode.current = 1.1;
	myNode.voltage = 3.3;
	myNode.light_sensor_value = 0;
	myNode.current_mode = MODE_MANUAL;
	struct time_set timeSetting = {
		.h_start_0	= 6,
		.m_start_0	= 0,
		.h_stop_0		= 17,
		.m_stop_0		= 59,
		
		.h_start_50	= 18,
		.m_start_50	= 0,
		.h_stop_50	= 18,
		.m_stop_50	= 59,
		
		.h_start_75	= 19,
		.m_start_75	= 0,
		.h_stop_75		= 21,
		.m_stop_75		= 59,
		
		.h_start_100	= 22,
		.m_start_100	= 0,
		.h_stop_100		= 5,
		.m_stop_100		= 59
	};
	curTime.hour		= 6;
	curTime.minutes = 15;
	curTime.second	= 30;
	while(1)
	{
		myNode.light_sensor_value = 4096 - read_adc(3);
		ping_to_wdt();
		
		lcd_goto_xy(0, 0);
		memset(buffer, 0,50);
		sprintf(buffer, "ID: %d", NODE_ID);
		lcd_display(buffer);
		
		lcd_goto_xy(0, 1);
		memset(buffer, 0,50);
		sprintf(buffer, (myNode.current_mode == MODE_AUTO) ? "Auto, %d" : "Manual, %d", myNode.illuminance);
		lcd_display(buffer);
		
		if(rx_sig)
		{
			get_data();
		}
		if(myNode.current_mode == MODE_AUTO)
			auto_mode(myNode.light_sensor_value, &myNode.illuminance);
		else if(myNode.current_mode == MODE_MANUAL)
			manual_mode(timeSetting, &myNode.illuminance);
		if(myNode.illuminance != last_dt)
			pwm_setDutyCycle(myNode.illuminance);
		last_dt = myNode.illuminance;
		delay_ms(1000);
	}
}
void EXTI2_IRQHandler(void)
{
	rx_sig = 1;
	EXTI->PR |= (1UL<<2);
}
static void get_data(void)
{
	uint8_t r;
	memset(tx_buff, 0, sizeof(tx_buff));
	sprintf(tx_buff, "%d %d %.2f %.2f %d", myNode.light_sensor_value, myNode.illuminance, myNode.voltage, myNode.current, myNode.current_mode);
	memset(recv_data, 0, 50);
	r = LoRa_receive(&myLoRa, recv_data, PACKET_SIZE);
	if(r)
	{
		handler_rx_data(recv_data);
	}
	rx_sig = 0;
}
static void manual_mode(struct time_set foo, uint8_t *duty_cycle)
{
	/* Illuminance 0% */
	if(foo.h_start_0 < foo.h_stop_0)
	{
		if(foo.h_start_0 <= curTime.hour && foo.h_stop_0 >= curTime.hour && foo.m_start_0 <= curTime.minutes && foo.m_stop_0 >= curTime.minutes)
			*duty_cycle = 0;
	}
	else if(foo.h_start_0 > foo.h_stop_0)
	{
		if(( (foo.h_start_0 <= curTime.hour) && (foo.m_start_0 <= curTime.minutes) ) || ( (foo.h_stop_0 >= curTime.hour) && (foo.m_stop_0 >= curTime.minutes) ))
			*duty_cycle = 0;
	}
	else
	{
		if(foo.m_start_0 < foo.m_stop_0)
		{
			if(foo.h_start_0 <= curTime.hour && foo.h_stop_0 >= curTime.hour && foo.m_start_0 <= curTime.minutes && foo.m_stop_0 >= curTime.minutes)
				*duty_cycle = 0;
		}
		else
		{
			if(( (foo.h_start_0 <= curTime.hour) && (foo.m_start_0 <= curTime.minutes) ) || ( (foo.h_stop_0 >= curTime.hour) && (foo.m_stop_0 >= curTime.minutes) ))
			*duty_cycle = 0;
		}
	}
	
	/* Illuminance 50% */
	if(foo.h_start_50 < foo.h_stop_50)
	{
		if(foo.h_start_50 <= curTime.hour && foo.h_stop_50 >= curTime.hour && foo.m_start_50 <= curTime.minutes && foo.m_stop_50 >= curTime.minutes)
			*duty_cycle = 50;
	}
	else if(foo.h_start_50 > foo.h_stop_50)
	{
		if(( (foo.h_start_50 <= curTime.hour) && (foo.m_start_50 <= curTime.minutes) ) || ( (foo.h_stop_50 >= curTime.hour) && (foo.m_stop_50 >= curTime.minutes) ))
			*duty_cycle = 50;
	}
	else
	{
		if(foo.m_start_50 < foo.m_stop_50)
		{
			if(foo.h_start_50 <= curTime.hour && foo.h_stop_50 >= curTime.hour && foo.m_start_50 <= curTime.minutes && foo.m_stop_50 >= curTime.minutes)
				*duty_cycle = 50;
		}
		else
		{
			if(( (foo.h_start_50 <= curTime.hour) && (foo.m_start_50 <= curTime.minutes) ) || ( (foo.h_stop_50 >= curTime.hour) && (foo.m_stop_50 >= curTime.minutes) ))
			*duty_cycle = 50;
		}
	}
	
	/* Illuminance 75% */
	if(foo.h_start_75 < foo.h_stop_75)
	{
		if(foo.h_start_75 <= curTime.hour && foo.h_stop_75 >= curTime.hour && foo.m_start_75 <= curTime.minutes && foo.m_stop_75 >= curTime.minutes)
			*duty_cycle = 75;
	}
	else if(foo.h_start_75 > foo.h_stop_75)
	{
		if(( (foo.h_start_75 <= curTime.hour) && (foo.m_start_75 <= curTime.minutes) ) || ( (foo.h_stop_75 >= curTime.hour) && (foo.m_stop_75 >= curTime.minutes) ))
			*duty_cycle = 75;
	}
	else
	{
		if(foo.m_start_75 < foo.m_stop_75)
		{
			if(foo.h_start_75 <= curTime.hour && foo.h_stop_75 >= curTime.hour && foo.m_start_75 <= curTime.minutes && foo.m_stop_75 >= curTime.minutes)
				*duty_cycle = 75;
		}
		else
		{
			if(( (foo.h_start_75 <= curTime.hour) && (foo.m_start_75 <= curTime.minutes) ) || ( (foo.h_stop_75 >= curTime.hour) && (foo.m_stop_75 >= curTime.minutes) ))
			*duty_cycle = 75;
		}
	}
	
	/* Illuminance 100% */
	if(foo.h_start_100 < foo.h_stop_100)
	{
		if(foo.h_start_100 <= curTime.hour && foo.h_stop_100 >= curTime.hour && foo.m_start_100 <= curTime.minutes && foo.m_stop_100 >= curTime.minutes)
			*duty_cycle = 100;
	}
	else if(foo.h_start_100 > foo.h_stop_100)
	{
		if(( (foo.h_start_100 <= curTime.hour) && (foo.m_start_100 <= curTime.minutes) ) || ( (foo.h_stop_100 >= curTime.hour) && (foo.m_stop_100 >= curTime.minutes) ))
			*duty_cycle = 100;
	}
	else
	{
		if(foo.m_start_100 < foo.m_stop_100)
		{
			if(foo.h_start_100 <= curTime.hour && foo.h_stop_100 >= curTime.hour && foo.m_start_100 <= curTime.minutes && foo.m_stop_100 >= curTime.minutes)
				*duty_cycle = 100;
		}
		else
		{
			if(( (foo.h_start_100 <= curTime.hour) && (foo.m_start_100 <= curTime.minutes) ) || ( (foo.h_stop_100 >= curTime.hour) && (foo.m_stop_100 >= curTime.minutes) ))
			*duty_cycle = 100;
		}
	}
}
static void auto_mode(uint32_t light_sensor, uint8_t *duty_cycle)
{
	if(light_sensor <= 2000)
		*duty_cycle = 50;
	else
		*duty_cycle = 0;
}
