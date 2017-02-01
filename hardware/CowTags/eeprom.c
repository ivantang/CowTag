/*
 * eeprom.c
 *
 *  Created on: Jan 24, 2017
 *      Author: Ivan
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Timestamp.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h> //i2c


#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/I2C.h> //i2c library
#include <ti/drivers/UART.h> //UART library

/* Example/Board Header files */
#include "Board.h"
#include <debug.h>
#include <stdint.h>
#include <assert.h>

#include "eeprom.h"
#include "boolean.h"
#include <IIC.c>
#include <IIC.h>

/**constants*/
#define TASKSTACKSIZE		1024	//i2c

/**structures*/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

bool eeprom_init() {
	// start at beginning of memory addresses
	currentAddress = 0x0000;

	// start eeprom thread
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)eeprom_testCapacity, &taskParams, NULL);

	return true;
}

bool eeprom_write(uint8_t bytes[]) {
	assertAddress(currentAddress);

	unsigned bytesLength = sizeof(*bytes) / sizeof(bytes[0]);

	// write a single byte
	if (bytesLength == 1) {
		uint8_t writeByte[] = {(currentAddress >> 8), currentAddress & 0xFF, bytes[0]};
		writeI2CArray(BOARD_24LC256, writeByte);
		currentAddress++;

	// write numerous bytes at once
	} else if (bytesLength > 1) {
		unsigned i;
		uint8_t writePage[sizeof(bytes) + 2];  // +2 for the address to write
		for(i = 2 ; i < sizeof(writePage) ; i++) {
			writePage[i] = bytes[i];
		}

		// address to start writing at
		writePage[0] = (currentAddress >> 8);
		writePage[1] = currentAddress & 0xFF;
		currentAddress += bytesLength;

		writeI2CArray(BOARD_24LC256, writePage);

	// no bytes to write!
	} else {
		System_printf("ERR: attempting to write empty bytes @ %x\n", currentAddress);
		System_flush();

		return false;
	}

	// delay
	Task_sleep(100000 / Clock_tickPeriod);

	return true;
}

/* Sets the address pointer on EEPROM to addrHigh and addrLow
 * Returns the value in addrHigh addrLow
 * Increments address pointer to next entry
 * */
static uint8_t eeprom_readAddress(uint8_t addrHigh, uint8_t addrLow) {
	assertAddress(currentAddress);
	return readEEPROMaddress(BOARD_24LC256, addrHigh, addrLow);
}

/* Returns value referenced by address pointer on EEPROM
 * Does NOT change address pointer
*/
static uint8_t eeprom_readCurrentAddress() {
	assertAddress(currentAddress);
	return readEEPROMaddress(BOARD_24LC256, (currentAddress >> 8), currentAddress & 0xFF);
}

/* TODO:
 * Sets the address pointer on EEPROM to addrHigh and addrLow
 * Returns the value in addrHigh addrLow as well as the next readNum #'s of values in array
 * Increments address pointer to next entry after the last read
 * */
static void eeprom_readPage(uint8_t addrHigh, uint8_t addrLow, uint8_t rxBuffer[]) {
	uint8_t			txBuffer[2];

	txBuffer[0] = addrHigh;
	txBuffer[1] = addrLow;

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

	I2C_Params_init(&params);
	params.transferMode = I2C_MODE_BLOCKING;
	params.bitRate = I2C_400kHz;

	i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 2;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 6;//sizeof(*rxBuffer) / sizeof(rxBuffer[0]);
	i2cTransaction.slaveAddress = BOARD_24LC256;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}

	if(I2C_transfer(handle, &i2cTransaction) == NULL){
		System_abort("I2C Transfer Failed at Read Random\n");
	}

	I2C_close(handle);
}

bool eeprom_clear() {
	// clear all bytes
	uint16_t i = 0x0000;
	uint8_t emptyByte[] = {0x00};
	for (i = 0; i < MAX_EEPROM_ADDRESS; ++i) {
		assertAddress(currentAddress);
		eeprom_write(emptyByte);
	}

	// reset address pointer
	currentAddress = MIN_EEPROM_ADDRESS;

	return true;
}

/*** diagnostic ***/

bool eeprom_isEmpty() {
	if (currentAddress == MIN_EEPROM_ADDRESS) {
		return true;
	} else {
		return false;
	}
}

bool eeprom_isFull() {
	if (currentAddress < MAX_EEPROM_ADDRESS) {
		return true;
	} else {
		return false;
	}
}

bool eeprom_canFit(uint8_t byte) {
	if (currentAddress + SAMPLE_SIZE <= MAX_EEPROM_ADDRESS) {
		return true;
	} else {
		return false;
	}
}

bool eeprom_canFitMany(uint8_t bytes[]) {
	if (currentAddress + (sizeof(bytes) * SAMPLE_SIZE) <= MAX_EEPROM_ADDRESS) {
		return true;
	} else {
		return false;
	}
}

int eeprom_spaceLeft() {
	uint16_t remainingAddresses = MAX_EEPROM_ADDRESS - currentAddress;
	return remainingAddresses / SAMPLE_SIZE;
}

/*** assertions ***/

void assertAddress(uint16_t address) {
	assert(address < MAX_EEPROM_ADDRESS && address >= MIN_EEPROM_ADDRESS);
}

/*** tests ***/

void eeprom_testCapacity() {
	uint8_t input[] = { 0x00 };
	uint8_t received = 0x00;
	uint16_t testsize = 0x0020;
	bool writeWorked[32];
	bool readWorked[32];

	currentAddress = MIN_EEPROM_ADDRESS;

	for (; currentAddress < testsize;) {
		input[0] = currentAddress & 0xff;
		writeWorked[currentAddress - 1] = eeprom_write(input);

		received = eeprom_readAddress((currentAddress - 1) >> 8, (currentAddress - 1) & 0xFF);

		if (received == input[0]) {
			readWorked[currentAddress - 1] = true;
		} else {
			readWorked[currentAddress - 1] = false;
		}
	}

	// count successful writes/reads
	unsigned write_wins = 0;
	unsigned write_loses = 0;
	unsigned read_wins = 0;
	unsigned read_loses = 0;

	uint16_t i;
	for (i = 0x00; i < testsize; ++i) {
		if (writeWorked[i] == true) {
			write_wins++;
		} else {
			write_loses++;
		}

		if (readWorked[i] == true) {
			read_wins++;
		} else {
			read_loses++;
		}
	}

	System_printf("[eeprom_testCapacity]\n");
	System_printf("WRITE -> Wins: %d, Loses: %d\n", write_wins, write_loses);
	System_printf("READ - > Wins: %d, Loses: %d\n", read_wins, read_loses);
	System_flush();
}

void eeprom_testPageWrite() {
//	currentAddress = 0x0000;
//
//	uint8_t input[] = { 'a', 'b', 'c', 'd', 'e', 'f' };
//
//	bool success = eeprom_write(input);
//
//	System_printf("win?  %d\n", success);
//
//	currentAddress = 0x0000;
//
//	uint8_t received = eeprom_readAddress(currentAddress >> 8, currentAddress & 0xFF);
//	System_printf("Read? %c\n", received);
//	System_flush();
//
//	++currentAddress;
//
//	received = eeprom_readAddress(currentAddress >> 8, currentAddress & 0xFF);
//	System_printf("Read? %c\n", received);
//	System_flush();
//
//	++currentAddress;
//
//	received = eeprom_readAddress(currentAddress >> 8, currentAddress & 0xFF);
//	System_printf("Read? %c\n", received);
//	System_flush();
//
//	++currentAddress;
//
//	received = eeprom_readAddress(currentAddress >> 8, currentAddress & 0xFF);
//	System_printf("Read? %c\n", received);
//	System_flush();
}
