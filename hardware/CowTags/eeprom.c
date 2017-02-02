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
	Task_construct(&task0Struct, (Task_FuncPtr)eeprom_testWriteReadSample, &taskParams, NULL);

	return true;
}

bool eeprom_write(uint8_t bytes[], int numBytes) {
	assertAddress(currentAddress);

	unsigned i = 0;
	while (i < numBytes) {
		bool writeSuccess = false;

		// retry if read does not correlate with write
		while (!writeSuccess) {
			uint8_t writeByte[] = {(currentAddress >> 8), currentAddress & 0xFF, *bytes};

			writeI2CArray(BOARD_24LC256, writeByte);

			// delay
			Task_sleep(100000 / Clock_tickPeriod);

			// validate write
			uint8_t received[1];
			eeprom_readAddress(currentAddress >> 8, currentAddress & 0xff, 1, received);

			if (*received == *bytes) {
				writeSuccess = true;
			}
		}

		++i;
		++currentAddress;
		++bytes;
	}

	return true;
}

/* Sets the address pointer on EEPROM to addrHigh and addrLow
 * Returns the value in addrHigh addrLow
 * Increments address pointer to next entry
 * */
void eeprom_readAddress(uint8_t addrHigh, uint8_t addrLow, int numBytes, uint8_t *buf) {
	assertAddress(currentAddress);

	unsigned i = 0;
	while (i < numBytes) {
		buf[i] = readEEPROMaddress(BOARD_24LC256, addrHigh + (i >> 8), addrLow + (i & 0xff));
		++i;

		// delay
		Task_sleep(100000 / Clock_tickPeriod);
	}
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
	uint8_t emptyByte[] = { 0x00 };

	for (i = 0; i < MAX_EEPROM_ADDRESS; ++i) {
		assertAddress(currentAddress);
		eeprom_write(emptyByte, 1);
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

void eeprom_testWriteReadSample() {
	System_printf("[eeprom_testWriteReadSample]\n");
	System_flush();

	// 18 byte test sample
	uint8_t sample[] = {
			0x11, 0x22, 0x33, 0x44, 0x55,
			0x66, 0x77, 0x88, 0x99, 0xaa,
			0xbb, 0xcc, 0xdd, 0xee, 0xff,
			0x1f, 0x2e, 0x3d
	};

	currentAddress = MIN_EEPROM_ADDRESS;
	eeprom_write(sample, SAMPLE_SIZE);
	currentAddress = MIN_EEPROM_ADDRESS;

	uint8_t received[18];
	eeprom_readAddress(currentAddress >> 8, currentAddress & 0xff, 18, received);

	int matches = 0;
	int i;
	for (i = 0; i < 18; i++) {
		if (sample[i] == received[i]) {
			matches++;
		}
	}

	System_printf("write/read matches = %d / 18!\n", matches);

	System_flush();
}

void eeprom_testClearMemory() {
	System_printf("[eeprom_testClearMemory]\n");
	System_flush();

	bool result = eeprom_clear();

	if (result) {
		System_printf("eeprom cleared!\n");
		System_flush();
	} else {
		System_printf("ERR: eeprom not cleared correctly\n");
		System_flush();
	}
}

void eeprom_testValidateMemory() {
	System_printf("[eeprom_testValidateMemory]\n");
	System_flush();

	// test settings
	uint8_t input[] = { 0x22 };
	uint16_t testsize = 0x000f;
	uint8_t received[1];

	// tally bad bytes
	unsigned badWrites = 0;
	unsigned badReads = 0;

	// reset memory pointer
	currentAddress = MIN_EEPROM_ADDRESS;

	for (; currentAddress < testsize;) {
		// check write
		if (!eeprom_write(input, 1)) {
			badWrites++;
		}

		// check read
		eeprom_readAddress((currentAddress - 1) >> 8, (currentAddress - 1) & 0xff, 1, received);

		if (received[0] != input[0]) {
			++badReads;
		}
	}

	// count successful writes/reads
	System_printf("Bad Writes = %d / %d\n", badWrites, testsize);
	System_printf("Bad Reads = %d / %d\n", badReads, testsize);
	System_flush();
}
