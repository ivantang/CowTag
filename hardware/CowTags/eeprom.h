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
const uint8_t SAMPLE_SIZE = 0x10; // 16 bytes
const uint8_t BOARD_24LC256_1 = 0x50;	//slave address for first eeprom (a1a2a3 = 000)
const uint8_t BOARD_24LC256_2 = 0x51; //slave address for second eeprom

// members
uint16_t currentAddress;
uint16_t lastAddress;

// functions
void testEEPROM();
bool eeprom_init();  // connect with 24LC256 chip
bool eeprom_write(uint8_t byte);  // write to next address
bool eeprom_writeMany(uint8_t byte[]); // write to next sequential addresses
bool eeprom_clear(); // set all entires in memory to 0x00
static uint8_t EEPROMReadPage(uint8_t slaveaddr, uint8_t addrHigh, uint8_t addrLow, uint8_t readNum);
static uint8_t EEPROMReadCurrent(uint8_t board_address);
static uint8_t EEPROMReadRandom(uint8_t slaveaddr, uint8_t addrHigh, uint8_t addrLow);

// diagnostic
bool eeprom_isEmpty();
bool eeprom_isFull();
bool eeprom_canFit(uint8_t byte);
bool eeprom_canFitMany(uint8_t bytes[]);
int eeprom_spaceLeft();  // number of samples that can fit

// assertions
void assertAddress(uint16_t address);


#endif /* EEPROM_H_ */
