/*
	TITLE: implement lora mesh
	DESCRIPTION: soil & air monitoring
	AUTHOR: Dinh Huu Nam
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "systick_delay.h"
#include "lcd_hehe.h"
#include "adc_nam.h"
#include "dht11_nam.h"
#include "wdt_nam.h"
#include "lora.h"
#include "mesh_nam.h"
static volatile uint8_t rx_sig = 0;
static double temperature, humidity;
static double analog = 0;
char tx_buff[40];
extern uint8_t cache_buffer[PACKET_SIZE];
static volatile int RSSI;
struct LoRa_Setup myLoRa;
extern void SysTick_Handler(void);
static double soil_mesurement(uint8_t channel);
static void get_data(void);
static uint8_t recv_data[50];
void EXTI2_IRQHandler(void);
int main(void)
{
	char buff_dht[255];
	char buff_analog[255];
	myLoRa = newLora();
	lcd_init();
	while (LoRa_init(&myLoRa) != LORA_OK)
	{
		lcd_goto_xy(0,0);
		lcd_display("Please Wait...!");
		delay_ms(100);
	}
	dht11_init();
	adc_init();
	clear_lcd();
	lcd_goto_xy(0, 0);
	lcd_display("Init success!");
	LoRa_startReceiving(&myLoRa);
	delay_ms(2000);
	watchdog_init();
	memset(cache_buffer, 97, PACKET_SIZE);
	while(1)
	{
		read_data(&temperature, &humidity);
		analog = soil_mesurement(3);
		memset(buff_analog, '\0', 255);
		memset(buff_dht, '\0', 255);
		sprintf(buff_analog, "%s: %.2f", "S moisture", analog);
		sprintf(buff_dht, "%c: %.1f, %c %.1f", 'T', temperature, 'H', humidity);
		lcd_goto_xy(0, 0);
		lcd_display(buff_dht);
		lcd_goto_xy(0, 1);
		lcd_display(buff_analog);
		ping_to_wdt();
		if(rx_sig)
		{
			get_data();
		}
		delay_ms(1000);
	}
}
static double soil_mesurement(uint8_t channel)
{
	double temp;
	temp = read_adc(channel);
	temp = (double)temp*100;
	temp = (double)temp/4095;
	return temp;
}
void EXTI2_IRQHandler(void)
{
	rx_sig = 1;
	EXTI->PR |= (1UL<<2);
}
static void get_data(void)
{
	uint8_t r;
//	RSSI = LoRa_getRSSI();
	memset(tx_buff, '\0', 40);
	sprintf(tx_buff, "%.2f %.2f %.2f", temperature, humidity, analog);
	memset(recv_data, 0, 50);
	r = LoRa_receive(&myLoRa, recv_data, PACKET_SIZE);
	if(r)
	{
		handler_rx_data(recv_data);
	}
	rx_sig = 0;
}
