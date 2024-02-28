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
#include "lcd.h"
#include "adc.h"
#include "watchdog.h"
#include "lora.h"
#include "mesh.h"
#include "pwm.h"
#include "light_control.h"
const uint32_t NODE_ID = 20021165;
static volatile uint8_t rx_sig = 0;
char tx_buff[50];
struct time curTime;
struct time_set timeSetting;
struct LoRa_Setup myLoRa;
static void get_data(void);
static uint8_t recv_data[PACKET_SIZE];
void EXTI2_IRQHandler(void);
struct LoRa_node myNode;

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
	myNode.current_mode = MODE_AUTO;
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
		/* start task 1 */
		if(rx_sig)
		{
			get_data();
		}
		/* end task 1 */
		/* start task 2 */
		if(myNode.current_mode == MODE_AUTO)
			myNode.illuminance = getDutyCycle_Auto(myNode.light_sensor_value);
		else if(myNode.current_mode == MODE_MANUAL)
			myNode.illuminance = getDutyCycle_Manual(timeSetting);
		if(myNode.illuminance != last_dt)
			pwm_setDutyCycle(myNode.illuminance);
		last_dt = myNode.illuminance;
		/* end task 2 */
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
	memset(recv_data, 0, PACKET_SIZE);
	r = LoRa_receive(&myLoRa, recv_data, PACKET_SIZE);
	if(r)
	{
		handler_rx_data(recv_data);
	}
	rx_sig = 0;
}

