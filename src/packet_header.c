/*
 * packet_header.c
 *
 *  Created on: Nov 7, 2018
 *      Author: alzakariyamq
 */

#include "packet_header.h"
#include <stdlib.h>

// Polynomial: x^8+x^2+x+1, as per the CRC-8-CCITT standard. (do not include the highest degree polynomial: X^8)
#define CRC8_POLYNOMIAL ((1<<2)|(1<<1)|(1<<0))
// lookup table of precomputed crc8 bytes for fast CRC8 computation
static uint8_t crc8_lookuptable[256] = {0};

static void compute_crc8_lookup_table(uint8_t *lookup_table, uint8_t polynomial);
static uint8_t compute_crc8_byte(uint8_t byte, uint8_t polynomial);

/**
 * module init. Pre-computes and caches the CRC8 lookup table for quick CRC8 calculation
 */
void ph_init() {
	compute_crc8_lookup_table(crc8_lookuptable, CRC8_POLYNOMIAL);
}

/**
 * initiates a PacketHeader object
 * @param src Source address of the packet header
 * @param dest Destination address of the packet header
 * @param crc_flag if true, compute CRC. else set to 0x00
 * @param msg A message buffer representing the message to put within the header.
 * @param size The size of the message. Max 255 bytes.
 */
PacketHeader* ph_create(uint8_t src, uint8_t dest, bool crc_flag, const void* msg, uint8_t size) {
	PacketHeader *pkt = (PacketHeader*)malloc(sizeof(PacketHeader));
	pkt->src = src;
	pkt->dest = dest;
	pkt->length = size;
	pkt->crc_flag = crc_flag;
	char *copiedMsg = (char*)malloc(sizeof(size));
	memcpy(copiedMsg, msg, size);
	pkt->msg = copiedMsg;
	if (pkt->crc_flag)
		pkt->crc8_fcs = ph_compute_crc8(pkt);
	else
		pkt->crc8_fcs = 0xAA;
}

/**
 * Parses the header from a buffer message.
 * if the format of the header is invalid, NULL is returned.
 * Invalid format is:
 * 	- buffer size includes message length + sizeof(PacketHeader) - sizeof(void*)
 * 	- crc_flag is 0, but crc8_fcs is not 0xAA
 * @param buf The buffer to parse
 * @return a malloc'd PacketHeader or NULL if invalid format
 */
PacketHeader* ph_parse(const void* buf, unsigned int size) {
	// buffer must contain at least absolute size of PacketHeader
	if (size < sizeof(PacketHeader))
		return NULL;
	// parse initial parameters
	PacketHeader *pkt = (PacketHeader*)malloc(sizeof(PacketHeader));
	pkt->synch = ((PacketHeader*)buf)->synch;
	pkt->ver = ((PacketHeader*)buf)->ver;
	pkt->src = ((PacketHeader*)buf)->src;
	pkt->dest = ((PacketHeader*)buf)->dest;
	pkt->length = ((PacketHeader*)buf)->length;
	pkt->crc_flag = ((PacketHeader*)buf)->crc_flag;

	// make sure that the size accounts for the length of msg + 1 (CRC byte)
	if (size < sizeof(PacketHeader) - sizeof(void*) + pkt->length + 1) {
		free(pkt);
		return NULL;
	}

	// parse message
	char *msg = (char*)malloc(pkt->length);
	memcpy(msg, &((PacketHeader*)buf)->msg, pkt->length);
	pkt->msg = msg;

	// make sure that the CRC field makes sense semantically. (must be 0xAA if no CRC)
	if (!pkt->crc_flag && ((uint8_t*)buf)[size-1] != 0xAA) {
		free(pkt);
		return NULL;
	}

	// parse CRC field
	pkt->crc8_fcs = ((uint8_t*)buf)[size-1];

	return pkt;
}

/**
 * frees PacketHeader malloc'd structs by freeing
 * their message content and the PacketHeader pointer.
 */
void ph_free(PacketHeader* pkt) {
	free(pkt->msg);
	free(pkt);
}


/**
 * Computes the CRC8 checksum for the given packet header.
 */
uint8_t ph_compute_crc8(const PacketHeader* pkt) {
	uint8_t crc = 0;
	for (int byte=0; byte<pkt->length; byte++) {
		crc ^= ((uint8_t*)pkt->msg)[byte];
		crc = crc8_lookuptable[crc];
	}
	return crc;
}

/**
 * Confirms that the packet is valid through its crc8 field. If crc8_flag is false, this is automatically true.
 */
bool ph_confirm_crc8(const PacketHeader* pkt);

/**
 * computes the CRC8 for one byte. This should only be used to populate crc8_lookup_table
 */
static uint8_t compute_crc8_byte(uint8_t byte, uint8_t polynomial) {
	uint8_t crc = 0;
	// for each input bit
	for (int bit=0; bit<8; bit++) {
		// for each iteration, the bit shifts through the registers
		crc <<= 1;

		// load the next input in
		crc &= ~(0x01);
		crc |= (byte & (1<<bit))>>bit;

		// each register associated to a polynomial xors the value just shifted into it
		crc ^= polynomial;
	}
	return crc;
}

static void compute_crc8_lookup_table(uint8_t *lookup_table, uint8_t polynomial) {
	for (int i=0; i<256; i++) {
		lookup_table[i] = compute_crc8_byte(i, polynomial);
	}
}
