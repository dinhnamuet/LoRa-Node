#ifndef __SPI_DRIVER_H__
#define __SPI_DRIVER_H__
#include <stdlib.h>
#include <string.h>
#include "systick_delay.h"
#define F_CLK_SPI_2M 32
#define F_CLK_SPI_9M 16
#define set(port,bit) port|=(1U<<bit)
#define reset(port,bit) port&=~(unsigned int)(1U<<bit)
#define toggle(port,bit) port^=(1U<<bit)

#define SS_ENABLE		reset(GPIOA->ODR, 4);
#define SS_DISABLE		set(GPIOA->ODR, 4);

void spi_master_init(void);
void spi_send_data(uint8_t data);
void spi_send_single_byte(uint8_t data);
uint8_t spi_receive(void);
void spi_send_buffer(uint8_t *tx_buffer);
void spi_send_buffer_soft(uint8_t *tx_buffer);

#endif
