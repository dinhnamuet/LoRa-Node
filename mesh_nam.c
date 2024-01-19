#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mesh_nam.h"
#include "lora.h"
extern struct LoRa_Setup myLoRa;
static uint8_t cache_buffer[PACKET_SIZE];
extern char tx_buff[40];
struct LoRa_packet default_packet(void)
{
	struct LoRa_packet foo;
	foo.uid = NODE_ID;
	foo.TTL = 15;
	return foo;
}
static void format_pkt(struct LoRa_packet pkt, uint8_t *buff)
{
	int i = 0;
	memset(buff, 0, PACKET_SIZE);
	// UID
	buff[0] = pkt.uid>>24;
	buff[1] = (pkt.uid>>16) & 0xFF;
	buff[2] = (pkt.uid>>8) & 0xFF;
	buff[3] = pkt.uid & 0xFF;
	//Destination ID
	buff[4] = pkt.destination_id>>24;
	buff[5] = (pkt.destination_id>>16) & 0xFF;
	buff[6] = (pkt.destination_id>>8) & 0xFF;
	buff[7] = pkt.destination_id & 0xFF;
	//Data length
	buff[8] = pkt.data_length;
	//Data
	for(i = 0; i<pkt.data_length; i++)
	{
		buff[BASE_DATA+i] = pkt.data[i];
	}
	//TTL
	buff[BASE_TTL] = pkt.TTL;
}
static void mesh_send_pkt(struct LoRa_packet pkt)
{
	uint8_t to_send[PACKET_SIZE];
	format_pkt(pkt, to_send);
	memset(cache_buffer, 0, PACKET_SIZE);
	memcpy(cache_buffer, to_send, PACKET_SIZE);
	LoRa_transmit(&myLoRa, to_send, PACKET_SIZE, 1000);
}
static void mesh_send_pkt_no_cache(struct LoRa_packet pkt)
{
	uint8_t to_send[PACKET_SIZE];
	format_pkt(pkt, to_send);
	LoRa_transmit(&myLoRa, to_send, PACKET_SIZE, 1000);
}
void handler_rx_data(uint8_t *buff)
{
	uint32_t i;
	struct LoRa_packet foo;
	struct LoRa_packet responses;
	foo.uid = (uint32_t)(buff[0]<<24 | buff[1]<<16 | buff[2]<<8 | buff[3]);
	foo.destination_id = (uint32_t)(buff[4]<<24 | buff[5]<<16 | buff[6]<<8 | buff[7]);
	foo.TTL = buff[BASE_TTL];
	foo.data_length = buff[8];
	foo.data = (char *)calloc(foo.data_length, sizeof(uint8_t));
	for(i = 0; i<foo.data_length; i++)
	{
		 foo.data[i] = buff[BASE_DATA+i];
	}
	/* handler data after unpack */
	if(foo.destination_id != NODE_ID && foo.uid != NODE_ID)
	{
		if(strncmp((char *)cache_buffer, (char *)buff, (PACKET_SIZE - 1))!= 0 && foo.TTL > 0)
		{
			foo.TTL --;
			mesh_send_pkt(foo);
		}
	}
	else if (foo.destination_id == NODE_ID)
	{
		if(strncmp((char *)cache_buffer, (char *)buff, 49)!= 0)
		{
			memset(cache_buffer, 0, PACKET_SIZE);
			memcpy(cache_buffer, buff, PACKET_SIZE);
			if(strncmp(foo.data, "request", strlen("request")) == 0)
			{
				responses = default_packet();
				responses.destination_id = GATEWAY_ID;
				responses.data_length = (uint8_t)strlen(tx_buff);
				responses.data = tx_buff;
				mesh_send_pkt_no_cache(responses);
			}
		}
	}
	else
	{
		//do nothing
	}
	free(foo.data);
}
