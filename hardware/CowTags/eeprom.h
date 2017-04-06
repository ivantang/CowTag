/*
 * eeprom.h
 *
 * This driver interacts with the 24LC256 eeprom chip.
 * It can read/write and function as a generator.
 *
 *  Created on: Jan 24, 2017
 *      Author: Ivan
 */

#ifndef EEPROM_H_
#define EEPROM_H_

/***** Includes *****/
#include <stdint.h>
#include <stdbool.h>
#include <radioProtocol.h>

// members
extern uint16_t eeprom_currentAddress;
extern uint16_t eeprom_lastAddress;
extern bool eeprom_hasWrapped;

/***** prototypes *****/
bool eeprom_write(struct sampleData *data);  // write to next address
bool eeprom_getNext(struct sampleData *data);
void eeprom_reset(void); // reset memory address pointer to 0x0000

#endif /* EEPROM_H_ */
