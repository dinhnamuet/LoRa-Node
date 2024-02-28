#ifndef __LORA_MESH_H__
#define __LORA_MESH_H__
#include <stdint.h>
#define GATEWAY_ID 		20021163
#define BASE_DATA 		10
#define PACKET_SIZE 	60

#define GLOBALIP 		282002
#define MODE_MANUAL		6
#define MODE_AUTO		7
#define FIND_DATA		8
#define REQUEST_DATA	2
#define RESPONSE_DATA	3

#pragma pack(1)
struct LoRa_packet {
	uint32_t uid;
	uint32_t destination_id;
	uint8_t pkt_type;
	uint8_t data_length;
	char data[PACKET_SIZE - BASE_DATA];
};
struct time {
	int hour;
	int minutes;
	int second;
};
struct time_set
{
	int h_start_0;
	int h_stop_0;
	int m_start_0;
	int m_stop_0;

	int h_start_50;
	int h_stop_50;
	int m_start_50;
	int m_stop_50;

	int h_start_75;
	int h_stop_75;
	int m_start_75;
	int m_stop_75;

	int h_start_100;
	int h_stop_100;
	int m_start_100;
	int m_stop_100;
};
struct LoRa_node
{
	uint8_t current_mode;
	uint32_t light_sensor_value;
	uint8_t illuminance;
	float voltage;
	float current;
};
#pragma pack()
void handler_rx_data(uint8_t *buff);
#endif
