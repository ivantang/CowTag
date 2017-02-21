/*
 * IIC.c
 *
 *  Created on: Nov 24, 2016
 *      Author: Ivan
 */


/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/I2C.h> //i2c
#include <ti/drivers/UART.h> //i2c
#include <IIC.h>

/* Example/Board Header files */
#include <Board.h>
#include <debug.h>
#include <stdint.h>
#include <assert.h>

/**
 * write an array to a slave i2c device
 * the first element is the slave address
 *
 * @bytes[0]   slave address
 * @bytes[1..] data
 */
void writeI2CEEPROM(uint8_t slaveAddr, uint8_t bytes[]) {
	// 3 bytes per request
	uint8_t			txBuffer[3];
	uint8_t         rxBuffer[1];

	I2C_Transaction i2cTransaction;
	I2C_Handle 		handle;
	I2C_Params		params;

	// init clock speed and synch
	I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_100kHz;

    assert(sizeof(bytes) > 0);

    // load data
    int i;
    int arrayLen = sizeof(bytes);
    for (i = 0; i < arrayLen; i++){
    	txBuffer[i] = bytes[i];
    }

    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 3;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    i2cTransaction.slaveAddress = slaveAddr;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_printf("ABORT\n");
		System_abort("Error Initializing I2C for Transmitting\n");
	}

	for(i = 0; i < retryI2CCount ; i++){
		if(I2C_transfer(handle, &i2cTransaction) != NULL) break;
		if(verbose_i2c && i == retryI2CCount-1){
			System_printf("I2C Transfer timed out\n");
			System_flush();
		}
	}

	I2C_close(handle);
}

uint8_t readI2CEEPROM(uint8_t slaveaddr, uint8_t addrHigh, uint8_t addrLow) {
	uint8_t			txBuffer[2];
	uint8_t			rxBuffer[1];

	txBuffer[0] = addrHigh;
	txBuffer[1] = addrLow;

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

	params.transferMode = I2C_MODE_BLOCKING;
	params.bitRate = I2C_400kHz;
	I2C_Params_init(&params);

	i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 2;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 1;
	i2cTransaction.slaveAddress = slaveaddr;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}

	// loop until eeprom is ready
	int i = 0;
	for(i = 0; i < retryI2CCount ; i++){
		if(I2C_transfer(handle, &i2cTransaction) != NULL) break;
		if(verbose_i2c && i == retryI2CCount-1){
			System_printf("I2C Transfer timed out\n");
			System_flush();
		}
	}

	I2C_close(handle);

	return rxBuffer[0];
}

//sends 8bit value to target address on target board address
//first 8 bits in txbuffer is address on hardware we want to write to
//seconds 8 bits in txbuffer is value we want to write
void writeI2CRegister(uint8_t board_address, uint8_t destination, uint8_t value){
	uint8_t			txBuffer[2];
	uint8_t         rxBuffer[1];

	I2C_Transaction i2cTransaction;
	I2C_Handle 		handle;
	I2C_Params		params;

	I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_400kHz;

    /*prepare data to send*/
    txBuffer[0] = destination;
    txBuffer[1] = value;

    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    i2cTransaction.slaveAddress = board_address; //0x18


	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		if(verbose_i2c) System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer
	int i = 0;
	for(i = 0; i < retryI2CCount ; i++){
		if(I2C_transfer(handle, &i2cTransaction) != NULL) break;
		if(verbose_i2c && i == retryI2CCount-1){
			System_printf("I2C Transfer timed out\n");
			System_flush();
		}
	}

	/*Deinitialized I2C */
	I2C_close(handle);

}

//similar to writeI2CRegister but instead takes arrays as arguments
void writeI2CRegisters(int8_t board_address, uint8_t destination[], uint8_t value[]){
	unsigned int 	i;
	uint8_t			txBuffer[sizeof(destination)+sizeof(value) + 2];
	uint8_t         rxBuffer[1];

	I2C_Transaction i2cTransaction;
	I2C_Handle 		handle;
	I2C_Params		params;

	I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_400kHz;

    /*check if destination and value arrays are same size*/
    assert(sizeof(destination) == sizeof(value));

    /*prepare data to send*/
    for(i = 0; i < sizeof(destination) + 1; i++){
    	txBuffer[2 * i + 0] = destination[i];
		txBuffer[2 * i + 1] = value[i];
    }

//print out tx details

//    for(i = 0; i < 10; i++)
//    	System_printf("%x ",txBuffer[i]);
//    System_printf("\ndestination %i\n",sizeof(destination));
//    System_printf("value %i\n",sizeof(value));

    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = (sizeof(txBuffer) + 1)*2;	//sizeof array gets -1 of actual size
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    i2cTransaction.slaveAddress = board_address; //0x18


	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}

	//do i2c transfer
	i = 0;
	for(i = 0; i < retryI2CCount ; i++){
		if(I2C_transfer(handle, &i2cTransaction) != NULL) break;
		if(verbose_i2c && i == retryI2CCount-1){
			System_printf("I2C Transfer timed out\n");
			System_flush();
		}
	}

	/*Deinitialized I2C */
	I2C_close(handle);
	if(verbose_i2c){
		System_printf("write closed\n");
	}
	System_flush();
}

//reads 8bit * 3 from target address
//100kHz is compatible with SMBUS
//this function is written to be used for the MLX90614 read format
uint32_t readI2CWord100kHz(uint8_t board_address, uint8_t address){
	uint8_t			txBuffer[1];
	uint8_t			rxBuffer[2];

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

	//load txBuffer
	txBuffer[0] = address;

    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_100kHz;

    i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 2;
	i2cTransaction.slaveAddress = board_address;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C at readI2CWord100kHz\n");
	}

	int i = 0;
	for(i = 0; i < retryI2CCount ; i++){
		if(I2C_transfer(handle, &i2cTransaction) != NULL) break;
		if(verbose_i2c && i == retryI2CCount-1){
			System_printf("I2C Transfer timed out\n");
			System_flush();
		}
	}
	I2C_close(handle);

    //First byte is lower 8 bits, second byte is high 8 bits
    return ((rxBuffer[0]) + (rxBuffer[1] <<8));
}

//input board address and address of register you want to read
//returns 8bit value in the register
uint8_t readI2CRegister(uint8_t board_address, uint8_t address){
	uint8_t			txBuffer[1] = {address};
	uint8_t			rxBuffer[1];

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_400kHz;

    i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 1;
	i2cTransaction.slaveAddress = board_address;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}

	int i = 0;
	for(i = 0; i < retryI2CCount ; i++){
		if(I2C_transfer(handle, &i2cTransaction) != NULL) break;
		if(verbose_i2c && i == retryI2CCount-1){
			System_printf("I2C Transfer timed out\n");
			System_flush();
		}
	}

    I2C_close(handle);

	return rxBuffer[0];
}

