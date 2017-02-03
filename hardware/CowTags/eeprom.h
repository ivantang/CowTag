/*
 * eeprom.h
 *
 *  Created on: Jan 24, 2017
 *      Author: Ivan
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>
#include <stdbool.h>

#define SAMPLE_SIZE 18

// constants
#define MAX_EEPROM_ADDRESS 0x7FFF
#define MIN_EEPROM_ADDRESS 0x0000
#define BOARD_24LC256 0x50	//slave address for first eeprom (a1a2a3 = 000)

// members
extern uint16_t eeprom_currentAddress;
extern uint16_t eeprom_lastAddress;
extern bool eeprom_hasWrapped;

// functions
bool eeprom_write(uint8_t byte[], int numBytes);  // write to next address
void eeprom_readAddress(uint8_t addrHigh, uint8_t addrLow, int numByte, uint8_t *buf);
void eeprom_getNext(uint8_t *buf);
void eeprom_reset(); // reset memory address pointer to 0x0000

// diagnostic
bool eeprom_isEmpty();
bool eeprom_isFull();
bool eeprom_canFit();
int eeprom_spaceLeft();  // number of samples that can fit

// assertions
void assertAddress(uint16_t address);
void assertSemaphore();


#endif /* EEPROM_H_ */
