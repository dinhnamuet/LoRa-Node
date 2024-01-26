#ifndef __LORA_MESH_H__
#define __LORA_MESH_H__
#include <stdint.h>
#define GATEWAY_ID 20021163
#define BASE_DATA 9
#define BASE_TTL 49
#define PACKET_SIZE 50
/*
Data format:
	ID | destination  | data_length | data | TTL
	4				4								1					40		1
*/
#pragma pack(1)
struct LoRa_packet {
	uint32_t uid;
	uint32_t destination_id;
	uint8_t TTL;
	uint8_t data_length;
	char *data;
};
#pragma pack()
struct LoRa_packet default_packet(void);
void handler_rx_data(uint8_t *buff);
void mesh_send_pkt_no_cache(struct LoRa_packet pkt);
#endif
