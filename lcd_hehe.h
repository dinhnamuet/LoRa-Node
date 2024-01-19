#ifndef __LCD_HEHE_H__
#define __LCD_HEHE_H__
#include <stdint.h>

#define SET(port,bit)			port|=(1U<<bit)
#define RESET(port,bit) 	port&=~(1U<<bit)
#define TOGGLE(port,bit)	port^=(1U<<bit)
#define COMMAND						0
#define DATA							1

#pragma pack(1)
struct LCD {
	/* GPIOB for control LCD */
	uint8_t RS:4;
	uint8_t RW:4;
	uint8_t EN:4;
	uint8_t D0:4;
	uint8_t D1:4;
	uint8_t D2:4;
	uint8_t D3:4;
	uint8_t D4:4;
	uint8_t D5:4;
	uint8_t D6:4;
	uint8_t D7;
};
#pragma pack()
void lcd_init(void);
void lcd_display(char *data);
void lcd_goto_xy(uint8_t x, uint8_t y);
void clear_lcd(void);
void free_lcd(void);
#endif
