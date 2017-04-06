/*
 * eeprom.c
 *
 *  Created on: Jan 24, 2017
 *      Author: Ivan
 */

/***** Includes *****/
#include "global_cfg.h"
#include <radioProtocol.h>
#include <eeprom.h>
#include <serialize.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h> //i2c library

/* Example/Board Header files */
#include <Board.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/* Drivers */
#include <ti/drivers/I2C.h> //i2c
#include <IIC.h>

/***** Defines *****/
#define BOARD_24LC256 0x50	//slave address for first eeprom (a1a2a3 = 000)

/***** Variable Declarations *****/
uint16_t eeprom_currentAddress = MIN_EEPROM_ADDRESS;
uint16_t eeprom_lastAddress = MIN_EEPROM_ADDRESS;
bool eeprom_semaphoreInit = false;
bool eeprom_hasWrapped = false;
Semaphore_Struct eepromSem;
Semaphore_Handle eepromSemHandle;

/***** Prototypes *****/
void eeprom_readAddress(uint8_t addrHigh, uint8_t addrLow, int numByte, uint8_t *buf);
/* diagnostic */
bool eeprom_isEmpty();
bool eeprom_canFit();
int eeprom_spaceLeft();  // number of samples that can fit
/* assertions */
void assertAddress(uint16_t address);
void assertSemaphore();

/***** Function Definition *****/
bool eeprom_write(struct sampleData *data) {
	if(verbose_eeprom){System_printf("Starting eepromWrite\n");System_flush();}
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
			writeI2CEEPROM(BOARD_24LC256, writeByte);

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

	if(verbose_eeprom){System_printf("Finsihed eepromWrite\n");System_flush();}
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
		buf[i] = readI2CEEPROM(BOARD_24LC256, addrHigh + (i >> 8), addrLow + (i & 0xff));
		++i;
	}
}

bool eeprom_getNext(struct sampleData *data) {
	if(verbose_eeprom){System_printf("Starting eepromGetNext\n");System_flush();}
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
		if (eeprom_lastAddress + SAMPLE_SIZE <= eeprom_currentAddress) {
			eeprom_readAddress(eeprom_lastAddress >> 8, eeprom_lastAddress & 0xff, SAMPLE_SIZE, buf);
			eeprom_lastAddress += SAMPLE_SIZE;

			// convert bytes to sensorPacket struct
			unserializePacket(data, buf);

		} else {
			eeprom_currentAddress = eeprom_lastAddress = MIN_EEPROM_ADDRESS;
			if(verbose_eeprom){System_printf("Finished eepromGetNext\n");System_flush();}
			return true;  // DONE
		}
	}
	if(verbose_eeprom){System_printf("Finished eepromGetNext\n");System_flush();}
	return false;  // NOT DONE
}

void eeprom_reset() {
	if(verbose_eeprom){System_printf("Reseting EEPROM\n");System_flush();}
	eeprom_currentAddress = MIN_EEPROM_ADDRESS;
	eeprom_lastAddress = MIN_EEPROM_ADDRESS;
	if(verbose_eeprom){System_printf("Reset COMPLETE\n");System_flush();}
}

/* diagnostic */
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

/* assertions */
void assertAddress(uint16_t address) {
	assert(address <= MAX_EEPROM_ADDRESS);
}

void assertSemaphore() {
	if(verbose_eeprom){System_printf("Asserting EEPROM SEMAPHORE\n");System_flush();}
	if (!eeprom_semaphoreInit) {
		eeprom_semaphoreInit = true;

		Semaphore_Params semParam;
		Semaphore_Params_init(&semParam);
		Semaphore_construct(&eepromSem, 1, &semParam);
		eepromSemHandle = Semaphore_handle(&eepromSem);
	}
	if(verbose_eeprom){System_printf("finished asserting EEPROM SEMAPHORE\n");System_flush();}
}
