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
	uint8_t length;
	uint8_t crc_flag;
	void *msg;
	uint8_t crc8_fcs;
}PacketHeader;

void ph_init();
PacketHeader* ph_create(uint8_t src, uint8_t dest, bool crc_flag, const void* msg, uint8_t size);
void* ph_createBuf(unsigned int *bufSize, uint8_t src, uint8_t dest, bool crc_flag, const void* msg, uint8_t size);
PacketHeader* ph_parse(const void* buf, unsigned int size);
void* ph_serialize(PacketHeader *pkt, unsigned int *size);
void ph_free(PacketHeader*);
uint8_t ph_compute_crc8(void *msg, unsigned int size);


#endif /* PACKET_HEADER_H_ */
