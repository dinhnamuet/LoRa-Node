/*
	TITLE: implement lora mesh
	DESCRIPTION: light monitoring system
	AUTHOR: Dinh Huu Nam
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "delay.h"
#include "lcd.h"
#include "adc.h"
#include "watchdog.h"
#include "lora.h"
#include "mesh.h"
#include "pwm.h"
#include "light_control.h"
#include "motion.h"

#include "FreeRTOS.h"
#include "task.h"

extern struct LoRa_packet packet_queue[QUEUE_SIZE];
extern int queue_ptr;

const uint32_t NODE_ID = 20021164;
static volatile uint8_t rx_sig = 0;
char tx_buff[50];
struct time curTime;
struct time_set timeSetting;
struct LoRa_Setup myLoRa;
static uint8_t recv_data[PACKET_SIZE];
void EXTI2_IRQHandler(void);
struct LoRa_node myNode;

static void mainThread(void *args);
static void dataHandling(void *args);
static void sendData(void *args);

int main(void)
{
	myLoRa = newLora();
		delay_init();
	lcd_init();
	while (LoRa_init(&myLoRa) != LORA_OK)
	{
		lcd_goto_xy(0,0);
		lcd_display("Please Wait...!");
		delay_ms(100);
	}
	motion_sensor_PC13_init();
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
	myNode.light_sensor_value = 4096 - read_adc(3);
	myNode.current_mode = MODE_AUTO;
	
	xTaskCreate(mainThread, (const char*) "processTask", 256, NULL, 4, NULL);
	xTaskCreate(dataHandling, (const char*) "RxTask", 1000, NULL, 1, NULL);
	xTaskCreate(sendData, (const char *)"TxTask", 1000, NULL, 1, NULL);

  vTaskStartScheduler();
	while(1);
}
void EXTI2_IRQHandler(void)
{
	uint8_t r = 0;
	memset(recv_data, 0, PACKET_SIZE);
	r = LoRa_receive(&myLoRa, recv_data, PACKET_SIZE);
	if(r)
		rx_sig = 1;
	EXTI->PR |= (1UL<<2);
}

static void mainThread(void *args)
{
	uint8_t last_dt = 0;
	char buffer[50];
	while(1)
	{
		ping_to_wdt();
		myNode.light_sensor_value = 4096 - read_adc(3);
		lcd_goto_xy(0, 0);
		memset(buffer, 0,50);
		sprintf(buffer, "ID: %d", NODE_ID);
		lcd_display(buffer);
		lcd_goto_xy(0, 1);
		memset(buffer, 0,50);
		sprintf(buffer, (myNode.current_mode == MODE_AUTO) ? "Auto, %d" : "Manual, %d", myNode.illuminance);
		lcd_display(buffer);
		if(myNode.current_mode == MODE_AUTO)
			myNode.illuminance = getDutyCycle_Auto(myNode.light_sensor_value);
		else if(myNode.current_mode == MODE_MANUAL)
			myNode.illuminance = getDutyCycle_Manual(timeSetting);
		if(myNode.illuminance != last_dt)
		{
			clear_lcd();
			pwm_setDutyCycle(myNode.illuminance);
		}
		last_dt = myNode.illuminance;
		vTaskDelay(800/portTICK_RATE_MS);
	}
}	

static void dataHandling(void *args)
{
	while(1)
	{
		if(rx_sig)
		{
			memset(tx_buff, 0, sizeof(tx_buff));
			sprintf(tx_buff, "%d %d %.2f %.2f %d", myNode.light_sensor_value, myNode.illuminance, myNode.voltage, myNode.current, myNode.current_mode);
			handler_rx_data(recv_data);
			rx_sig = 0;
		}
		vTaskDelay(800/portTICK_RATE_MS);
	}
}

static void sendData(void *args)
{
	while(1)
	{
		if(queue_ptr)
		{
			mesh_send_pkt(&myLoRa, packet_queue[0]);
			delete_packet_from_queue(0);
		}
		vTaskDelay(800/portTICK_RATE_MS);
	}
}
