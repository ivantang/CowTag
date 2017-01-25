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
#include <IIC.c>
#include <IIC.h>

/**constants*/
#define TASKSTACKSIZE		1024	//i2c

/**structures*/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*function definition
 * Creating a separate task for EEPROM as a means to test the memory component
 * */
void EEPROM_init(void){
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)testEEPROM, &taskParams, NULL);
}

/* this function is a temporary function which tests the eeprom  separately from other tasks */
void testEEPROM(){
	unsigned int	i;
	uint8_t controlbyte_write = 0x50;
	uint8_t addr_upper = 0x40;
	uint8_t addr_lower = 0x80;
	uint8_t data = 0x01;

	//random read write
	uint8_t writeByte[] = {addr_upper,addr_lower,data};

	//making the array
	uint8_t writePage[66];
	for(i = 2 ; i < sizeof(writePage) ; i++){
		writePage[i] = data;
		data++;
	}
	writePage[0] = addr_upper;
	writePage[1] = addr_lower;

	writeI2CArray(controlbyte_write,writeByte);
	Task_sleep(100000 / Clock_tickPeriod);						//required to sleep in between write/read
	EEPROMReadRandom(controlbyte_write,addr_upper,addr_lower);
}

/*
bool eeprom_init() {
	// initialize hardware
	init24LC256();

	// start at beginning of memory addresses
	currentAddress = 0x000;

	return true;
}*/

bool eeprom_write(uint8_t byte) {
	assertAddress(currentAddress);
	/* writeI2CByte(currentAddress, byte); */

	// set current memory stack pointer
	currentAddress++;

	return true;
}

bool eeprom_writeMany(uint8_t byte[]) {
	// if all bytes can fit, write them
	if (eeprom_canFitMany(byte)) {
		int numberOfBytes = sizeof(byte);
		int i = 0;

		for (i = 0; i < numberOfBytes; ++i) {
			assertAddress(currentAddress);
			/* writeI2CByte(currentAddress, byte[i]); */

			// set current memory stack pointer
			currentAddress++;
		}

	// if all bytes can't fit, write until the memory is full
	// then continue writing from MIN_EEPROM_ADDRESS
	} else {
		int numberOfBytes = sizeof(byte);
		int i = 0;

		for (i = 0; i < numberOfBytes; ++i) {
			assertAddress(currentAddress);

			if (eeprom_canFit(byte[i])) {
				/* writeI2CByte(currentAddress, byte[i]); */
			} else {
				// memory is full, start back at first address
				currentAddress = MIN_EEPROM_ADDRESS;
				/* writeI2CByte(currentAddress, byte[i]); */
			}

			// set current memory stack pointer
			currentAddress++;
		}
	}

	return true;
}

/* Sets the address pointer on EEPROM to addrHigh and addrLow
 * Returns the value in addrHigh addrLow
 * Increments address pointer to next entry
 * */
static uint8_t EEPROMReadRandom(uint8_t slaveaddr, uint8_t addrHigh, uint8_t addrLow) {
	uint8_t			txBuffer[2];
	uint8_t			rxBuffer[1];

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
	i2cTransaction.readCount = 1;
	i2cTransaction.slaveAddress = slaveaddr;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}

	if(I2C_transfer(handle, &i2cTransaction) == NULL){
		System_abort("I2C Transfer Failed at Read Random\n");
	}

	I2C_close(handle);

	System_printf("random read at 0x%2x%2x: 0x%x \n",addrHigh,addrLow, rxBuffer[0]);
	System_flush();

	return rxBuffer[0];
}

/* Returns value referenced by address pointer on EEPROM
 * Does NOT change address pointer
*/
static uint8_t EEPROMReadCurrent(uint8_t board_address){
	uint8_t			txBuffer[0];
	uint8_t			rxBuffer[1];

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_400kHz;

    i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 0;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 1;
	i2cTransaction.slaveAddress = board_address;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}
	else {
		//if(verbose)System_printf("I2C Initialized!\n");
	}
	System_flush();

    I2C_transfer(handle, &i2cTransaction);

    System_printf("read current 0x%x\n",rxBuffer[0]);
    System_flush();

    I2C_close(handle);
    return rxBuffer[0];
	//if(verbose)	System_printf("read closed\n");
	System_flush();
}

/* TODO:
 * Sets the address pointer on EEPROM to addrHigh and addrLow
 * Returns the value in addrHigh addrLow as well as the next readNum #'s of values in array
 * Increments address pointer to next entry after the last read
 * */
static uint8_t EEPROMReadPage(uint8_t slaveaddr, uint8_t addrHigh, uint8_t addrLow, uint8_t readNum) {
	uint8_t			txBuffer[2];
	uint8_t			rxBuffer[64];
	uint8_t			i;

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
	i2cTransaction.readCount = 64;
	i2cTransaction.slaveAddress = slaveaddr;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}

	if(I2C_transfer(handle, &i2cTransaction) == NULL){
		System_abort("I2C Transfer Failed at Read Random\n");
	}

	I2C_close(handle);

	for(i = 0 ; i < 64 ; i++){
		System_printf("Read 0x%x \n", rxBuffer[i]);
	}

	System_flush();

	return rxBuffer[0];
}


bool eeprom_clear() {
	currentAddress = MIN_EEPROM_ADDRESS;

	uint16_t i = 0x0000;
	for (i = 0; i < MAX_EEPROM_ADDRESS; ++i) {
		assertAddress(currentAddress);
		/* writeI2CByte(currentAddress, 0x00); */
	}

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
