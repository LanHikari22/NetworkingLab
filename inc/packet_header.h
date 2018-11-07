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

typedef struct {
	uint8_t synch;
	uint8_t ver;
	uint8_t src;
	uint8_t dest;
	uint8_t crc_flag;
	uint8_t *msg;
	uint8_t crc8_fcs;
}PacketHeader;

PacketHeader* ph_parse(const void* buf, unsigned int size);
void ph_free(PacketHeader*);
PacketHeader* ph_create(uint8_t src, uint8_t dest, bool crc_flag, const void* msg, uint8_t size);
uint8_t ph_compute_crc8(const PacketHeader*);
bool ph_confirm_crc8(const PacketHeader*);


#endif /* PACKET_HEADER_H_ */
