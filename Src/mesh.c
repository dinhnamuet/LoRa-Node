#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mesh.h"
#include "lora.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"

struct LoRa_packet packet_queue[QUEUE_SIZE];
int queue_ptr = 0;

extern const uint32_t NODE_ID;
extern struct LoRa_Setup myLoRa;
extern struct time curTime;
extern struct time_set timeSetting;
extern struct LoRa_node myNode;
extern char tx_buff[50];
static uint32_t request_cached	= 0;
static uint32_t response_cached = 0;
static uint32_t manual_cached		= 0;
static uint32_t auto_cached		= 0;
static int add_packet_to_queue(struct LoRa_packet packet)
{
	int i;
	for(i = 0; i<QUEUE_SIZE; i++)
	{
		if(packet_queue[i].uid == packet.uid)
			return -1;
	}
	packet_queue[queue_ptr] = packet;
	queue_ptr = (queue_ptr == QUEUE_SIZE - 1) ? 0 : (queue_ptr+1);
	return 0;
}
void delete_packet_from_queue(uint8_t idx)
{
	int i;
	for(i = idx; i<QUEUE_SIZE - 1; i++)
	{
		packet_queue[i] = packet_queue[i+1];
	}
	memset(&packet_queue[QUEUE_SIZE - 1], 0, sizeof(packet_queue[QUEUE_SIZE - 1]));
	--queue_ptr;
}
static void format_pkt(struct LoRa_packet pkt, uint8_t *buff)
{
	int i = 0;
	memset(buff, 0, PACKET_SIZE);
	// Type
	buff[0] = pkt.pkt_type;
	// UID
	buff[1] = pkt.uid >> 24;
	buff[2] = (pkt.uid >> 16) & 0xFF;
	buff[3] = (pkt.uid >> 8) & 0xFF;
	buff[4] = pkt.uid & 0xFF;
	// Destination ID
	buff[5] = pkt.destination_id >> 24;
	buff[6] = (pkt.destination_id >> 16) & 0xFF;
	buff[7] = (pkt.destination_id >> 8) & 0xFF;
	buff[8] = pkt.destination_id & 0xFF;
	// Data length
	buff[9] = pkt.data_length;
	// Data
	for (i = 0; i < pkt.data_length; i++)
	{
		buff[BASE_DATA + i] = pkt.data[i];
	}
}

int mesh_send_pkt(struct LoRa_Setup *_LoRa, struct LoRa_packet pkt)
{
	uint8_t read;
	int time_set;
	if(pkt.pkt_type == RESPONSE_DATA)
		time_set = 3000;
	else
		time_set = 1000;
	int mode = _LoRa->current_mode;
	LoRa_gotoMode(_LoRa, STANDBY_MODE);
	delay_ms(1);
	while(1)
	{
		LoRa_gotoMode(_LoRa, CAD);
		while(!(LoRa_read(RegIrqFlags)>>2 & 0x01))
		{
			delay_ms(1);
		}
		read = LoRa_read(RegIrqFlags);
		if(read & 0x01) // cad detected
		{
			LoRa_write(RegIrqFlags, 0xFF);
			vTaskDelay(time_set/portTICK_RATE_MS);
		}
		else
		{
			LoRa_write(RegIrqFlags, 0xFF);
			LoRa_gotoMode(&myLoRa, mode);
			break;
		}
	}
	uint8_t to_send[PACKET_SIZE];
	format_pkt(pkt, to_send);
	LoRa_transmit(&myLoRa, to_send, strlen((char *)to_send), 1000);
	return 0;
}
void handler_rx_data(uint8_t *buff)
{
	uint32_t i;
	uint8_t flag = 0;
	struct LoRa_packet foo;
	struct LoRa_packet newPacket;
	/* Unpack LoRa packet */
	foo.pkt_type = buff[0];
	foo.uid = (uint32_t)(buff[1] << 24 | buff[2] << 16 | buff[3] << 8 | buff[4]);
	foo.destination_id = (uint32_t)(buff[5] << 24 | buff[6] << 16 | buff[7] << 8 | buff[8]);
	foo.data_length = buff[9];
	memset(foo.data, 0, PACKET_SIZE - BASE_DATA);
	for (i = 0; i < foo.data_length; i++)
	{
		foo.data[i] = buff[BASE_DATA + i];
	}
	/* handler data after unpack */
	switch(foo.pkt_type)
	{
		case REQUEST_DATA:
			if(foo.destination_id == NODE_ID && request_cached != NODE_ID) //packet for me && pending packet
			{
				newPacket.uid = NODE_ID;
				newPacket.destination_id = GATEWAY_ID;
				newPacket.pkt_type = RESPONSE_DATA;
				newPacket.data_length = strlen(tx_buff);
				memset(newPacket.data, 0, sizeof(newPacket.data));
				strncpy(newPacket.data, tx_buff, strlen(tx_buff));
				add_packet_to_queue(newPacket);
				response_cached = newPacket.uid;
				sscanf(foo.data, "%d %d %d", &curTime.hour, &curTime.minutes, &curTime.second); //parse time from data
			}
			else if(request_cached != foo.destination_id && foo.destination_id != NODE_ID) // packet for another
			{
				for(i = 0; i < queue_ptr; i++)
				{
					if(packet_queue[i].pkt_type == RESPONSE_DATA && packet_queue[i].uid == foo.destination_id)
					{
						flag = 1;
						break;
					}
				}
				if(!flag)
					add_packet_to_queue(foo);
				sscanf(foo.data, "%d %d %d", &curTime.hour, &curTime.minutes, &curTime.second); //parse time from data
				flag = 0;
			}
			request_cached = foo.destination_id;
			break;
		case RESPONSE_DATA:
			if(foo.uid != response_cached)
			{
				for(i = 0; i < queue_ptr; i++)
				{
					if(packet_queue[i].pkt_type == REQUEST_DATA && packet_queue[i].destination_id == foo.uid)
					{
						delete_packet_from_queue(i);
						break;
					}
				}			
				add_packet_to_queue(foo);
			}
			response_cached = foo.uid;
			break;
		case MODE_MANUAL:
			if(foo.destination_id == NODE_ID && manual_cached != NODE_ID) //packet for me && pending packet
			{
				myNode.current_mode = MODE_MANUAL;
				sscanf(foo.data, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &timeSetting.h_start_0, &timeSetting.m_start_0, &timeSetting.h_stop_0, &timeSetting.m_stop_0,
				&timeSetting.h_start_50, &timeSetting.m_start_50, &timeSetting.h_stop_50, &timeSetting.m_stop_50,
				&timeSetting.h_start_75, &timeSetting.m_start_75, &timeSetting.h_stop_75, &timeSetting.m_stop_75,
				&timeSetting.h_start_100, &timeSetting.m_start_100, &timeSetting.h_stop_100, &timeSetting.m_stop_100);
			}
			else if(manual_cached != foo.destination_id && foo.destination_id != NODE_ID) // packet for another
			{
				add_packet_to_queue(foo);
			}
			manual_cached = foo.destination_id;
			break;
		case MODE_AUTO:
			if(foo.destination_id == NODE_ID && auto_cached != NODE_ID) //packet for me && pending packet
			{
				myNode.current_mode = MODE_AUTO;
			}
			else if(auto_cached != foo.destination_id && foo.destination_id != NODE_ID) // packet for another
			{
				add_packet_to_queue(foo);
			}
			auto_cached = foo.destination_id;
			break;
		default:
			break;
	}
}
