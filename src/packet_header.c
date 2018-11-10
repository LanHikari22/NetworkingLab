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
 * creates a PacketHeader object
 * @param src Source address of the packet header
 * @param dest Destination address of the packet header
 * @param crc_flag if true, compute CRC. else set to 0x00
 * @param msg A message buffer representing the message to put within the header
 * @param size The size of the message. Max 255 bytes
 */
void ph_create(PacketHeader *out, uint8_t src, uint8_t dest, bool crc_flag, const void* msg, uint8_t size) {
	out->synch = 0x55;
	out->ver = 0x01;
	out->src = src;
	out->dest = dest;
	out->length = size;
	out->crc_flag = crc_flag;
	memcpy(out->msg, msg, size);
	if (out->crc_flag)
		out->crc8_fcs = ph_compute_crc8(out->msg, out->length);
	else
		out->crc8_fcs = 0xAA;
}

/**
 * Parses the header from a buffer message.
 * if the format of the header is invalid, false is returned.
 * Invalid format is:
 *	- buffer too small to encode full content
 * 	- crc_flag is 0, but crc8_fcs is not 0xAA
 * @param buf The buffer to parse
 * @return parsing status
 */
bool ph_parse(PacketHeader *out, const void* buf, unsigned int size) {
	// buffer must at least contain a msg of 1 byte
	if (size < sizeof(PacketHeader) - PH_MSG_SIZE+1)
		return false;
	// parse initial parameters
	out->synch = ((PacketHeader*)buf)->synch;
	out->ver = ((PacketHeader*)buf)->ver;
	out->src = ((PacketHeader*)buf)->src;
	out->dest = ((PacketHeader*)buf)->dest;
	out->length = ((PacketHeader*)buf)->length;
	out->crc_flag = ((PacketHeader*)buf)->crc_flag;

	// make sure that the size of the buffer accounts for the message content
	if (size < sizeof(PacketHeader) - PH_MSG_SIZE + out->length)
		return false;

	// parse message
	memcpy(out->msg, &((PacketHeader*)buf)->msg, out->length);

	// make sure that the CRC field makes sense semantically. (must be 0xAA if no CRC)
	if (!out->crc_flag && ((uint8_t*)buf)[size-1] != 0xAA) {
		return false;
	}

	// parse CRC field
	out->crc8_fcs = ((uint8_t*)buf)[size-1];

	return true;
}


/**
 * Computes the CRC8 checksum for the a message.
 */
uint8_t ph_compute_crc8(void *msg, unsigned int size) {
	uint8_t crc = 0;
	for (int byte=0; byte<size; byte++) {
		crc ^= ((uint8_t*)msg)[byte];
		crc = crc8_lookuptable[crc];
	}
	return crc;
}

/**
 * computes the CRC8 for one byte. This should only be used to populate crc8_lookup_table
 * @param byte value to compute CRC8 of
 * @param polynomial excluding x^8 (C7), which is assumed. This can be x^2 + x^1 + 1 for example, which gives 0b111
 */
static uint8_t compute_crc8_byte(uint8_t byte, uint8_t polynomial) {
	uint8_t crc = byte;
	// for each input bit
	for (int bit=0; bit<8; bit++) {
		// if x^8 (C7) == 1, each register associated to a polynomial xors the value just shifted into it
		if ((crc & (1<<7)) != 0) {
			crc <<= 1; // C7 is xor fed into the crc
			crc ^= polynomial;
		}
		else {
			// for each iteration, the bit shifts through the registers
			crc <<= 1;
		}
	}
	return crc;
}

static void compute_crc8_lookup_table(uint8_t *lookup_table, uint8_t polynomial) {
	for (int i=0; i<256; i++) {
		lookup_table[i] = compute_crc8_byte(i, polynomial);
	}
}
