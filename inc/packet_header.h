/*
 * packet_header.h
 *
 *  Created on: Nov 7, 2018
 *      Author: alzakariyamq
 */

#ifndef PACKET_HEADER_H_
#define PACKET_HEADER_H_

#include <inttypes.h>
#include <stdbool.h>

#define PH_MSG_SIZE 255

typedef struct {
	uint8_t synch;
	uint8_t ver;
	uint8_t src;
	uint8_t dest;
	uint8_t length;
	uint8_t crc_flag;
	uint8_t msg[PH_MSG_SIZE];
	uint8_t crc8_fcs;
}PacketHeader;


void ph_init();
void ph_create(PacketHeader *out, uint8_t src, uint8_t dest, bool crc_flag, const void* msg, uint8_t size);
bool ph_parse(PacketHeader *out, const void* buf, unsigned int size);
uint8_t ph_compute_crc8(void *msg, unsigned int size);


#endif /* PACKET_HEADER_H_ */
