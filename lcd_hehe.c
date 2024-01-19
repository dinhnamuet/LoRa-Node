#include "lcd_hehe.h"
#include "systick_delay.h"
#include <string.h>
#include <stdlib.h>
#include "stm32f10x.h"
static struct LCD *my_lcd;
static void lcd_set_pin_value(uint8_t num, uint8_t value)
{
	if(value)
		SET(GPIOB->ODR, num);
	else
		RESET(GPIOB->ODR, num);
}
static void lcd_io_init(void)
{
	uint8_t i;
	/* GPIO B clock enable */
	SET(RCC->APB2ENR, 3);
	/* GPIO B output push-pull */
	GPIOB->CRL = 0;
	for(i = 0; i<31; i+=4)
	{
		SET(GPIOB->CRL, i);
		SET(GPIOB->CRL, (i+1));
	}
	GPIOB->CRH = 0;
	for(i = 0; i<31; i+=4)
	{
		SET(GPIOB->CRH, i);
		SET(GPIOB->CRH, (i+1));
	}
}
static void send_to_lcd(uint8_t data, uint8_t MODE)
{
	if(MODE == COMMAND)
		lcd_set_pin_value(my_lcd->RS, 0);
	else if(MODE == DATA)
		lcd_set_pin_value(my_lcd->RS, 1);
	delay_us(50);
	lcd_set_pin_value(my_lcd->D0, (data&0x01)?1:0);
	lcd_set_pin_value(my_lcd->D1, (data>>1&0x01)?1:0);
	lcd_set_pin_value(my_lcd->D2, (data>>2&0x01)?1:0);
	lcd_set_pin_value(my_lcd->D3, (data>>3&0x01)?1:0);
	lcd_set_pin_value(my_lcd->D4, (data>>4&0x01)?1:0);
	lcd_set_pin_value(my_lcd->D5, (data>>5&0x01)?1:0);
	lcd_set_pin_value(my_lcd->D6, (data>>6&0x01)?1:0);
	lcd_set_pin_value(my_lcd->D7, (data>>7&0x01)?1:0);
	lcd_set_pin_value(my_lcd->EN, 0);
	delay_us(10);
	lcd_set_pin_value(my_lcd->EN, 1);
	delay_us(10);
	lcd_set_pin_value(my_lcd->EN, 0);
}
void lcd_init(void)
{
	lcd_io_init();
	my_lcd = (struct LCD *)calloc(1, sizeof(struct LCD));
	my_lcd->RS = 0;
	my_lcd->RW = 1;
	my_lcd->EN = 10;
	my_lcd->D0 = 11;
	my_lcd->D1 = 5;
	my_lcd->D2 = 6;
	my_lcd->D3 = 7;
	my_lcd->D4 = 8;
	my_lcd->D5 = 9;
	my_lcd->D6 = 12;
	my_lcd->D7 = 13;
	/* init */
	send_to_lcd(0x0E, COMMAND);
	delay_ms(5);
	send_to_lcd(0x3C, COMMAND);
	delay_ms(5);
	send_to_lcd(0x0C, COMMAND);
	delay_ms(5);
	send_to_lcd(0x01, COMMAND);
	delay_ms(5);
	send_to_lcd(0x02, COMMAND);
	delay_ms(5);
}
void lcd_display(char *data)
{
	uint32_t i;
	for(i = 0; data[i] != '\0'; i++)
		send_to_lcd(data[i], DATA);
}
void lcd_goto_xy(uint8_t x, uint8_t y)
{
	unsigned char Address = 0;
	if(y == 0) 
		Address = 0x80 + x;
	if(y == 1)
		Address = 0xc0 + x;
	send_to_lcd(Address, COMMAND);
	delay_us(100);
}
void clear_lcd(void)
{
	send_to_lcd(0x01, COMMAND);
	delay_ms(5);
}
void free_lcd(void)
{
	free(my_lcd);
}
