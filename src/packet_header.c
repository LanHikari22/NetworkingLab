/*
 * packet_header.c
 *
 *  Created on: Nov 7, 2018
 *      Author: alzakariyamq
 */

#include "packet_header.h"
#include <stdlib.h>

/**
 * Parses the header from a buffer message.
 * if the format of the header is invalid, NULL is returned.
 * Invalid format is:
 * 	- message length adds up to more than buffer size.
 * 	- total header size is not equal to buffer size
 * @param buf The buffer to parse
 * @return a malloc'd PacketHeader or NULL if invalid format
 */
PacketHeader* ph_parse(const void* buf, unsigned int size) {

}

/**
 * frees PacketHeader malloc'd structs by freeing
 * their message content and the PacketHeader pointer.
 */
void ph_free(PacketHeader* header) {
	free(header->msg);
	free(header);
}

/**
 * initiates a PacketHeader object
 * @param src Source address of the packet header
 * @param dest Destination address of the packet header
 * @param crc_flag if true, compute CRC. else set to 0x00
 * @param msg
 */
PacketHeader* ph_create(uint8_t src, uint8_t dest, bool crc_flag, const void* msg, uint8_t size);

}

/**
 * Computes the CRC8 checksum for the given packet header.
 */
uint8_t ph_compute_crc8(const PacketHeader*);
bool ph_confirm_crc8(const PacketHeader*);
