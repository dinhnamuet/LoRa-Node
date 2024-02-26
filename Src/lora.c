#include "lora.h"
#include "spi_driver.h"
#include <stdlib.h>
#define SET(port,bit) port|=(1U<<bit)
#define RESET(port,bit) port&=~(unsigned int)(1U<<bit)
#define TOGGLE(port,bit) port^=(1U<<bit)
struct LoRa_Setup newLora(void)
{
	struct LoRa_Setup SX1278;
	SX1278.frequency							= 433;
	SX1278.spreadingFactor				= SF_7;
	SX1278.bandWidth							= BW_125_KHZ;
	SX1278.codingRate							= CR_4_5;
	SX1278.power									= POWER_20db;
	SX1278.overCurrentProtection	= 100;
	SX1278.preamble								= 8;
	return SX1278;
}
static void Lora_Reset(void)
{
	RESET(GPIOA->ODR, RESET_PIN);
	delay_ms(1);
	SET(GPIOA->ODR, RESET_PIN);
	delay_ms(100);
}
static void LoRa_readReg(uint8_t address, uint8_t *output)
{
	SS_ENABLE
	spi_send_data(address);
	spi_receive();
	spi_send_data(0x00);
	*output = spi_receive();
	SS_DISABLE
}
static void LoRa_writeReg(uint8_t address, uint8_t output)
{
	SS_ENABLE
	spi_send_data(address);
	spi_receive();
	spi_send_data(output);
	spi_receive();
	SS_DISABLE
}
static uint8_t LoRa_read(uint8_t address)
{
	uint8_t read_data;
	uint8_t addr;
	addr = address & ~(1U<<7);
	LoRa_readReg(addr, &read_data);
	return read_data;
}
static void LoRa_write(uint8_t address, uint8_t value)
{
	uint8_t addr;
	addr = address | (1U<<7);
	LoRa_writeReg(addr, value);
}
static void LoRa_gotoMode(struct LoRa_Setup *_LoRa, int mode)
{
	uint8_t read;
	uint8_t data;
	read = LoRa_read(RegOpMode);
	switch(mode)
	{
		case SLEEP_MODE:
			data = (read & 0xF8) | SLEEP_MODE;
			_LoRa->current_mode = SLEEP_MODE;
			break;
		case STANDBY_MODE:
			data = (read & 0xF8) | STANDBY_MODE;
			_LoRa->current_mode = STANDBY_MODE;
			break;
		case TRANSMIT_MODE:
			data = (read & 0xF8) | TRANSMIT_MODE;
			_LoRa->current_mode = TRANSMIT_MODE;
			break;
		case RXCONTINUOUS_MODE:
			data = (read & 0xF8) | RXCONTINUOUS_MODE;
			_LoRa->current_mode = RXCONTINUOUS_MODE;
			break;
		case RXSINGLE_MODE:
			data = (read & 0xF8) | RXSINGLE_MODE;
			_LoRa->current_mode = RXSINGLE_MODE;
			break;
		default:
			break;
	}
	LoRa_write(RegOpMode, data);
}
static void LoRa_setFrequency(int freq)
{
	uint8_t  data;
	uint32_t F;
	F = (uint32_t)(freq * 524288)>>5;
	
	data = (uint8_t)(F >> 16)&0xFF;
	LoRa_write(RegFrMsb, data);
	delay_ms(5);
	
	data = (uint8_t)(F >> 8)&0xFF;
	LoRa_write(RegFrMid, data);
	delay_ms(5);
	
	data = (uint8_t)(F >> 0)&0xFF;
	LoRa_write(RegFrLsb, data);
	delay_ms(5);
}
static void LoRa_setSpreadingFactor(int SF)
{
	uint8_t	data;
	uint8_t	read;

	if(SF>12)
		SF = 12;
	if(SF<7)
		SF = 7;

	read = LoRa_read(RegModemConfig2);
	delay_ms(10);

	data = (uint8_t)((SF << 4) | (read & 0x0F));
	LoRa_write(RegModemConfig2, data);
	delay_ms(10);
}
static void LoRa_setPower(uint8_t power)
{
	LoRa_write(RegPaConfig, power);
	delay_ms(10);
}
static void LoRa_setOCP(uint8_t current)
{
	uint8_t	OcpTrim = 0;

	if(current<45)
		current = 45;
	if(current>240)
		current = 240;

	if(current <= 120)
		OcpTrim = (current - 45)/5;
	else if(current <= 240)
		OcpTrim = (current + 30)/10;

	OcpTrim = OcpTrim | (1U << 5);
	LoRa_write(RegOcp, OcpTrim);
	delay_ms(10);
}
static void LoRa_setTOMsb_setCRCon(void)
{
	uint8_t read, data;
	read = LoRa_read(RegModemConfig2);
	data = read | 0x07;
	LoRa_write(RegModemConfig2, data);
	delay_ms(10);
}
static void LoRa_BurstWrite(uint8_t address, uint8_t *value)
{
	uint8_t addr;
	addr = address | (1U<<7);
	SS_ENABLE
	spi_send_data(addr);
	spi_receive();
	delay_ms(1);
	spi_send_buffer_soft(value);
	SS_DISABLE
}
static uint8_t LoRa_isvalid(struct LoRa_Setup *LoRa)
{
	(void)LoRa;
	return 1;
}
uint8_t LoRa_transmit(struct LoRa_Setup *_LoRa, uint8_t *data, uint8_t length, uint16_t timeout)
{
	uint8_t read;
	int mode = _LoRa->current_mode;
	LoRa_gotoMode(_LoRa, STANDBY_MODE);
	read = LoRa_read(RegFiFoTxBaseAddr);
	LoRa_write(RegFiFoAddPtr, read);
	LoRa_write(RegPayloadLength, length);
	LoRa_BurstWrite(RegFifo, data);
	LoRa_gotoMode(_LoRa, TRANSMIT_MODE);
	while(1)
	{
		read = LoRa_read(RegIrqFlags);
		if(read & (1U<<3))
		{
			LoRa_write(RegIrqFlags, 0xFF);
			LoRa_gotoMode(_LoRa, mode);
			return 1;
		}
		else
		{
			if(--timeout == 0)
			{
				LoRa_gotoMode(_LoRa, mode);
				return 0;
			}
		}
		delay_ms(1);
	}
}
void LoRa_startReceiving(struct LoRa_Setup *_LoRa)
{
	LoRa_gotoMode(_LoRa, RXCONTINUOUS_MODE);
}
uint8_t LoRa_receive(struct LoRa_Setup *_LoRa, uint8_t *data, uint8_t length)
{
	uint8_t i = 0;
	uint8_t read;
	uint8_t data_len;
	uint8_t min = 0;
	LoRa_gotoMode(_LoRa, STANDBY_MODE);
	read = LoRa_read(RegIrqFlags);
	if(read & (1U<<6))
	{
		LoRa_write(RegIrqFlags, 0xFF);
		data_len = LoRa_read(RegRxNbBytes);
		read = LoRa_read(RegFiFoRxCurrentAddr);
		LoRa_write(RegFiFoAddPtr, read);
		min = (length >= data_len)?data_len:length;
		memset(data, 0, 50);
		for(i = 0; i<min; i++)
			data[i] = LoRa_read(RegFifo);
	}
	LoRa_gotoMode(_LoRa, RXCONTINUOUS_MODE);
	return min;
}
int LoRa_getRSSI(void)
{
	uint8_t read;
	read = LoRa_read(RegPktRssiValue);
	return -164 + read;
}
uint16_t LoRa_init(struct LoRa_Setup *_LoRa)
{
	uint8_t    data;
	uint8_t    read;
	spi_master_init();
	/* lora pin init */
	//DIO0 rising interrupt:
	SET(RCC->APB2ENR, 0);
	RESET(GPIOA->CRL, 8);
	RESET(GPIOA->CRL, 9);
	SET(GPIOA->CRL, 10);
	RESET(GPIOA->CRL, 11);
	AFIO->EXTICR[0] &= ~(unsigned int)(0x0F<<8);
	SET(EXTI->IMR, DIO0_PIN);
	SET(EXTI->RTSR, DIO0_PIN);
	NVIC_SetPriority(EXTI2_IRQn, 0);
	NVIC_EnableIRQ(EXTI2_IRQn);
	
	//RESET PIN OUTPUT PUSH-PULL, HIGH-LEVEL OUTPUT:
	SET(GPIOA->CRL, 4);
	SET(GPIOA->CRL, 5);
	RESET(GPIOA->CRL, 6);
	RESET(GPIOA->CRL, 7);
	SET(GPIOA->ODR, RESET_PIN);
	
	Lora_Reset();
	
	if(LoRa_isvalid(_LoRa)){
		// goto sleep mode:
			LoRa_gotoMode(_LoRa, SLEEP_MODE);
			delay_ms(10);

		// turn on lora mode:
			read = LoRa_read(RegOpMode);
			delay_ms(10);
			data = read | 0x80;
			LoRa_write(RegOpMode, data);
			delay_ms(100);

		// set frequency:
			LoRa_setFrequency(_LoRa->frequency);

		// set output power gain:
			LoRa_setPower(_LoRa->power);

		// set over current protection:
			LoRa_setOCP(_LoRa->overCurrentProtection);

		// set LNA gain:
			LoRa_write(RegLna, 0x23);

		// set spreading factor, CRC on, and Timeout Msb:
			LoRa_setTOMsb_setCRCon();
			LoRa_setSpreadingFactor(_LoRa->spreadingFactor);

		// set Timeout Lsb:
			LoRa_write(RegSymbTimeoutL, 0xFF);

		// set bandwidth, coding rate and expilicit mode:
			// 8 bit RegModemConfig --> | X | X | X | X | X | X | X | X |
			//       bits represent --> |   bandwidth   |     CR    |I/E|
			data = LoRa_read(RegModemConfig1);
			data |= (uint8_t)((_LoRa->bandWidth << 4) | (_LoRa->codingRate << 1));
			LoRa_write(RegModemConfig1, data);

		// set preamble:
			LoRa_write(RegPreambleMsb, _LoRa->preamble >> 8);
			LoRa_write(RegPreambleLsb, (uint8_t)_LoRa->preamble);

		// DIO mapping:   --> DIO: RxDone
			read = LoRa_read(RegDioMapping1);
			data = read | 0x3F;
			LoRa_write(RegDioMapping1, data);

		// goto standby mode:
			LoRa_gotoMode(_LoRa, STANDBY_MODE);
			_LoRa->current_mode = STANDBY_MODE;
			delay_ms(10);

			read = LoRa_read(RegVersion);
			if(read == 0x12)
				return LORA_OK;
			else
				return LORA_NOT_FOUND;
	}
	else 
	{
		return LORA_UNAVAILABLE;
	}
}
