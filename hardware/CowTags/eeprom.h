/*
 * eeprom.h
 *
 *  Created on: Jan 24, 2017
 *      Author: Ivan
 */

#ifndef EEPROM_H_
#define EEPROM_H_


// constants
const uint16_t MAX_EEPROM_ADDRESS = 0x7FFF;
const uint16_t MIN_EEPROM_ADDRESS = 0x0000;
const uint8_t BOARD_24LC256 = 0x50;	//slave address for first eeprom (a1a2a3 = 000)
size_t SAMPLE_SIZE = 18; // 18 bytes

// members
uint16_t currentAddress;
uint16_t lastAddress;

// functions
bool eeprom_init();  // connect with 24LC256 chip
bool eeprom_write(uint8_t byte[], int numBytes);  // write to next address
bool eeprom_writeMany(uint8_t byte[]); // write to next sequential addresses
bool eeprom_clear(); // set all entires in memory to 0x00
static uint8_t eeprom_readCurrentAddress();
void eeprom_readAddress(uint8_t addrHigh, uint8_t addrLow, int numByte, uint8_t *buf);

// diagnostic
bool eeprom_isEmpty();
bool eeprom_isFull();
bool eeprom_canFit(uint8_t byte);
bool eeprom_canFitMany(uint8_t bytes[]);
int eeprom_spaceLeft();  // number of samples that can fit

// assertions
void assertAddress(uint16_t address);

// tests
void eeprom_testWriteReadSample();
void eeprom_testClearMemory();
void eeprom_testValidateMemory();
void eeprom_testPageWrite();


#endif /* EEPROM_H_ */
