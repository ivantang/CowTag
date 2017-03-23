/*
 * bootTimestamp.h
 *
 *  Created on: March 16th, 2017
 *      Author: Steven Hall
 *
 */

#ifndef BOOT_TIMESTAMP_H
#define BOOT_TIMESTAMP_H

#include <stdint.h>

// Set once at the very beginning in main()
uint64_t boot_timestamp;

uint32_t sleep_offset;

uint32_t TrueTimestamp();

#endif // BOOT_TIMESTAMP_H
