#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mesh.h"
#include "lora.h"
#include "systick_delay.h"
#define QUEUE_SIZE 4
static struct LoRa_packet packet_queue[QUEUE_SIZE];
static uint8_t queue_ptr = 0;
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
static uint32_t last_finding = 0;
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
static void delete_packet_from_queue(uint8_t idx)
{
	int i;
	for(i = idx; i<QUEUE_SIZE - 1; i++)
	{
		packet_queue[i] = packet_queue[i+1];
	}
	memset(&packet_queue[QUEUE_SIZE - 1], 0, sizeof(packet_queue[QUEUE_SIZE - 1]));
	--queue_ptr;
}
static uint8_t find_data_index(uint32_t id)
{
	int i;
	for(i = 0; i < QUEUE_SIZE; i++)
	{
		if(packet_queue[i].uid == id)
			return i;
	}
	return -1;
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
static void mesh_send_pkt(struct LoRa_packet pkt)
{
	uint8_t to_send[PACKET_SIZE];
	format_pkt(pkt, to_send);
	LoRa_transmit(&myLoRa, to_send, strlen((char *)to_send), 1000);
}
static void forward_packet(struct LoRa_packet pkt)
{
	mesh_send_pkt(pkt);
}
void handler_rx_data(uint8_t *buff)
{
	uint32_t i;
	uint8_t index;
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
				mesh_send_pkt(newPacket); //sending response
				add_packet_to_queue(newPacket);
				response_cached = newPacket.uid;
				sscanf(foo.data, "%d %d %d", &curTime.hour, &curTime.minutes, &curTime.second); //parse time from data
			}
			else if(request_cached != foo.destination_id && foo.destination_id != NODE_ID) // packet for another
			{
				forward_packet(foo);
				sscanf(foo.data, "%d %d %d", &curTime.hour, &curTime.minutes, &curTime.second); //parse time from data
			}
			request_cached = foo.destination_id;
			break;
		case RESPONSE_DATA:
			if(foo.uid != response_cached)
			{
				forward_packet(foo); //forward data
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
				forward_packet(foo);
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
				forward_packet(foo);
			}
			auto_cached = foo.destination_id;
			break;
		case FIND_DATA:
			//if(foo.destination_id == last_finding)
				//break;
			index = find_data_index(foo.destination_id);
			if(index < 0)
			{
				//forward_packet(foo);
				sscanf(foo.data, "%d %d %d", &curTime.hour, &curTime.minutes, &curTime.second); //parse time from data
				break;
			}
			else
			{
				mesh_send_pkt(packet_queue[index]);
				delete_packet_from_queue(index);
				response_cached = foo.destination_id;
			}
			sscanf(foo.data, "%d %d %d", &curTime.hour, &curTime.minutes, &curTime.second); //parse time from data
			last_finding = foo.destination_id;
			break;
		default:
			break;
	}
}
