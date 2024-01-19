#include "spi_driver.h"
void spi_master_init(void)
{
	set(RCC->APB2ENR, 0); // enable clock for AFIO
	set(RCC->APB2ENR, 12); //enable clock for spi1
	set(RCC->APB2ENR, 2); // enable clock for gpioA
	GPIOA->CRL |= (1U<<16)|(1U<<17)|(1U<<20)|(1U<<21)|(1U<<28)|(1U<<29);
	reset(GPIOA->CRL, 18);
	reset(GPIOA->CRL, 19); // PA4 output pushpull
	set(GPIOA->CRL, 23);
	reset(GPIOA->CRL, 22); //PA5 alternate function pushpull
	reset(GPIOA->CRL, 24);
	reset(GPIOA->CRL, 25);
	set(GPIOA->CRL, 26);
	reset(GPIOA->CRL, 27); //PA6 floating input
	reset(GPIOA->CRL, 30);
	set(GPIOA->CRL, 31); // PA7 Alternate function pushpull
	set(GPIOA->ODR, 4);
	/* ----------------------------------------------------- */
//	set(SPI1->CR2, 6); //RXNE
	/* ----------------------------------------------------- */
	set(SPI1->CR1, 2); //master mode
	reset(SPI1->CR1, 13); //disable CRC
	reset(SPI1->CR1, 11); //8bit data
	reset(SPI1->CR1, 10); //full duplex
	reset(SPI1->CR1, 7); //MSB first
	SPI1->CR1 |= F_CLK_SPI_9M;
	set(SPI1->CR2, 2);
	reset(SPI1->CR1, 0); //CPOL = 0
	reset(SPI1->CR1, 1); //CPHA = 0
	set(SPI1->CR1, 6); // SPI enable
}
void spi_send_data(uint8_t data)
{
	while(SPI1->SR & (1U<<7));
	SPI1->DR = data & 0xFF;
//	while(SPI1->SR & (1U<<7));
	while(!(SPI1->SR & (1U<<1)));
}
uint8_t spi_receive(void)
{
	while(SPI1->SR & (1U<<7));
	while(!(SPI1->SR & (1U<<0)));
	return (uint8_t)SPI1->DR;
}
void spi_send_single_byte(uint8_t data)
{
	SS_ENABLE
	spi_send_data(data);
	SS_DISABLE
}
void spi_send_buffer(uint8_t *tx_buffer)
{
	uint32_t i = 0;
	SS_ENABLE
	while(tx_buffer[i] != '\0')
	{
		spi_send_data(tx_buffer[i]);
		spi_receive();
		++i;
	}
	SS_DISABLE
}
void spi_send_buffer_soft(uint8_t *tx_buffer)
{
	uint32_t i = 0;
	while(tx_buffer[i] != '\0')
	{
		spi_send_data(tx_buffer[i]);
		spi_receive();
		++i;
	}
}
