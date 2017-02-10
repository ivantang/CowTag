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
#include <ti/sysbios/knl/Semaphore.h>
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
#include "serialize.h"
#include <IIC.c>
#include <IIC.h>

bool eeprom_semaphoreInit = false;
bool eeprom_hasWrapped = false;
uint16_t eeprom_currentAddress = MIN_EEPROM_ADDRESS;
uint16_t eeprom_lastAddress = MIN_EEPROM_ADDRESS;

// sync primitive for eeprom
Semaphore_Struct eepromSem;
Semaphore_Handle eepromSemHandle;

bool eeprom_write(struct sampleData *data) {
	assertAddress(eeprom_currentAddress);
	assertSemaphore();

	/* get eeprom access sempahore */
	Semaphore_pend(eepromSemHandle, BIOS_WAIT_FOREVER);

	if (!eeprom_canFit()) {
		eeprom_currentAddress = MIN_EEPROM_ADDRESS;
		eeprom_hasWrapped = true;
	}

	// serialize packet data to bytes
	uint8_t bytes[SAMPLE_SIZE];
	serializePacket(data, bytes);

	unsigned i = 0;
	while (i < SAMPLE_SIZE) {
		bool writeSuccess = false;
		int maxRetry = 3;
		int currentRetry = 0;

		// retry if read does not correlate with write
		while (!writeSuccess) {
			uint8_t writeByte[] = {(eeprom_currentAddress >> 8), eeprom_currentAddress & 0xFF, bytes[i]};
			writeI2Ceeprom(BOARD_24LC256, writeByte);

			// validate write
			uint8_t received[1];
			eeprom_readAddress(eeprom_currentAddress >> 8, eeprom_currentAddress & 0xff, 1, received);

			if (*received == bytes[i]) {
				writeSuccess = true;
			} else {
				currentRetry++;
				if (currentRetry >= maxRetry) {
					writeSuccess = true;
				}
			}
		}

		++i;
		++eeprom_currentAddress;
	}

	/* return eeprom access semaphore */
	Semaphore_post(eepromSemHandle);

	return true;
}

/* Sets the address pointer on EEPROM to addrHigh and addrLow
 * Returns the value in addrHigh addrLow
 * Increments address pointer to next entry
 * */
void eeprom_readAddress(uint8_t addrHigh, uint8_t addrLow, int numBytes, uint8_t *buf) {
	assertAddress(eeprom_currentAddress);
	assertSemaphore();

	unsigned i = 0;
	while (i < numBytes) {
		buf[i] = readEEPROMaddress(BOARD_24LC256, addrHigh + (i >> 8), addrLow + (i & 0xff));
		++i;
	}
}

bool eeprom_getNext(struct sampleData *data) {
	uint8_t buf[SAMPLE_SIZE];

	// has wrapped: start back from beginning to read ALL samples
	if (eeprom_hasWrapped) {
		eeprom_lastAddress = MIN_EEPROM_ADDRESS;
		eeprom_currentAddress = MAX_EEPROM_ADDRESS;
		eeprom_readAddress(eeprom_lastAddress >> 8, eeprom_lastAddress & 0xff, SAMPLE_SIZE, buf);
		eeprom_lastAddress += SAMPLE_SIZE;
		eeprom_hasWrapped = false;

		// convert bytes to sensorPacket struct
		unserializePacket(data, buf);

		// no wrapping: read samples from lastAddress to currentAddress
	} else {
		if (eeprom_lastAddress < eeprom_currentAddress) {
			eeprom_readAddress(eeprom_lastAddress >> 8, eeprom_lastAddress & 0xff, SAMPLE_SIZE, buf);
			eeprom_lastAddress += SAMPLE_SIZE;

			// convert bytes to sensorPacket struct
			unserializePacket(data, buf);

		} else {
			eeprom_currentAddress = eeprom_lastAddress = MIN_EEPROM_ADDRESS;
			return true;  // DONE
		}
	}

	return false;  // NOT DONE
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

void eeprom_reset() {
	eeprom_currentAddress = MIN_EEPROM_ADDRESS;
	eeprom_lastAddress = MIN_EEPROM_ADDRESS;
}

/*** diagnostic ***/

bool eeprom_isEmpty() {
	if (eeprom_currentAddress == MIN_EEPROM_ADDRESS) {
		return true;
	} else {
		return false;
	}
}

bool eeprom_canFit() {
	if (eeprom_currentAddress + SAMPLE_SIZE <= MAX_EEPROM_ADDRESS) {
		return true;
	} else {
		return false;
	}
}

int eeprom_spaceLeft() {
	uint16_t remainingAddresses = MAX_EEPROM_ADDRESS - eeprom_currentAddress;
	return remainingAddresses / SAMPLE_SIZE;
}

/*** assertions ***/

void assertAddress(uint16_t address) {
	assert(address <= MAX_EEPROM_ADDRESS);
}

void assertSemaphore() {
	if (!eeprom_semaphoreInit) {
		eeprom_semaphoreInit = true;

		Semaphore_Params semParam;
		Semaphore_Params_init(&semParam);
		Semaphore_construct(&eepromSem, 1, &semParam);
		eepromSemHandle = Semaphore_handle(&eepromSem);
	}
}
