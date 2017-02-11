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

/* Example/Board Header files */
#include <Board.h>
#include <debug.h>
#include <stdint.h>
#include <assert.h>

//callback function for i2c callback mode, currently not using blocking mode so not using this
//can implement this later when needed
void transferCallback(I2C_Handle handle, I2C_Transaction *transac, bool result)
{
    // Set length bytes
    if (result) {
        //transferDone = true;
        if(verbose_sensors) System_printf("Transaction complete\n");
    } else {
        // Transaction failed, act accordingly...
        System_abort("Transaction failed\n");
    }
    System_flush();
}

//sends 8bit value to target i2c board address
void writeI2C(uint8_t board_address, uint8_t value){
	uint8_t			txBuffer[1];
	uint8_t         rxBuffer[1];

	I2C_Transaction t_i2cTransaction;
	I2C_Handle 		t_handle;
	I2C_Params		t_params;

	I2C_Params_init(&t_params);
    t_params.transferMode = I2C_MODE_BLOCKING;
    t_params.bitRate = I2C_100kHz;

    /*prepare data to send*/
    txBuffer[0] = value;

    t_i2cTransaction.writeBuf = txBuffer;
    t_i2cTransaction.writeCount = 1;
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = board_address; //0x18


	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		if(verbose_i2c) System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer
	I2C_transfer(t_handle, &t_i2cTransaction);

	/*Deinitialized I2C */
	I2C_close(t_handle);
	//if(verbose_sensors)	System_printf("write closed\n");
	System_flush();
}

/**
 * write an array to a slave i2c device
 * the first element is the slave address
 *
 * @bytes[0]   slave address
 * @bytes[1..] data
 */
void writeI2Ceeprom(uint8_t slaveAddr, uint8_t bytes[]) {
	int numBytes = 3;  // num bytes per eeprom request
	uint8_t			txBuffer[numBytes];
	uint8_t         rxBuffer[1];

	I2C_Transaction t_i2cTransaction;
	I2C_Handle 		t_handle;
	I2C_Params		t_params;

	// init clock speed and synch
	I2C_Params_init(&t_params);
    t_params.transferMode = I2C_MODE_BLOCKING;
    t_params.bitRate = I2C_100kHz;

    assert(sizeof(bytes) > 0);

    // load data
    int i;
    int arrayLen = sizeof(bytes);
    for (i = 0; i < arrayLen; i++){
    	txBuffer[i] = bytes[i];
    }

    t_i2cTransaction.writeBuf = txBuffer;
    t_i2cTransaction.writeCount = numBytes;
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = slaveAddr;


	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}

//	if(I2C_transfer(t_handle, &t_i2cTransaction) == NULL){
//		System_abort("I2C Transfer Failed\n");
//	}
	while(I2C_transfer(t_handle, &t_i2cTransaction) == NULL);

	I2C_close(t_handle);
}

//sends 8bit value to target address on target board address
//first 8 bits in txbuffer is address on hardware we want to write to
//seconds 8 bits in txbuffer is value we want to write
void writeI2CRegister(uint8_t board_address, uint8_t destination, uint8_t value){
	uint8_t			txBuffer[2];
	uint8_t         rxBuffer[1];

	I2C_Transaction t_i2cTransaction;
	I2C_Handle 		t_handle;
	I2C_Params		t_params;

	I2C_Params_init(&t_params);
    t_params.transferMode = I2C_MODE_BLOCKING;
    t_params.bitRate = I2C_400kHz;

    /*prepare data to send*/
    txBuffer[0] = destination;
    txBuffer[1] = value;

    t_i2cTransaction.writeBuf = txBuffer;
    t_i2cTransaction.writeCount = 2;
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = board_address; //0x18


	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		if(verbose_i2c) System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer
	if(I2C_transfer(t_handle, &t_i2cTransaction)==NULL){
		System_abort("I2C failed at I2CWriteRegister()");
	}

	/*Deinitialized I2C */
	I2C_close(t_handle);
	if(verbose_i2c){
		System_printf("write closed\n");
	}
	System_flush();
}

//similar to writeI2CRegister but instead takes arrays as arguments
void writeI2CRegisters(int8_t board_address, uint8_t destination[], uint8_t value[]){
	unsigned int 	i;
	uint8_t			txBuffer[sizeof(destination)+sizeof(value) + 2];
	uint8_t         rxBuffer[1];

	I2C_Transaction t_i2cTransaction;
	I2C_Handle 		t_handle;
	I2C_Params		t_params;

	I2C_Params_init(&t_params);
    t_params.transferMode = I2C_MODE_BLOCKING;
    t_params.bitRate = I2C_400kHz;

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

    t_i2cTransaction.writeBuf = txBuffer;
    t_i2cTransaction.writeCount = (sizeof(txBuffer) + 1)*2;	//sizeof array gets -1 of actual size
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = board_address; //0x18


	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		if(verbose_i2c) System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer
	I2C_transfer(t_handle, &t_i2cTransaction);

	/*Deinitialized I2C */
	I2C_close(t_handle);
	if(verbose_i2c){
		System_printf("write closed\n");
	}
	System_flush();
}

//reads 8bit * 3 from target address
uint32_t readI2CWord100kHz(uint8_t board_address, uint8_t address){
	uint8_t			txBuffer[1] = {address};
	uint8_t			rxBuffer[3];

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_100kHz;

    i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 3;
	i2cTransaction.slaveAddress = board_address;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}
	else {
		//if(verbose_sensors)System_printf("I2C Initialized!\n");
	}
	System_flush();

    I2C_transfer(handle, &i2cTransaction);

    if(verbose_i2c){
    	System_printf("rxBuffer: 0x%x%x%x read from 0x%x\n",rxBuffer[0],rxBuffer[1],rxBuffer[2],address);
        System_flush();
    }

    I2C_close(handle);

	if(verbose_i2c){
		System_printf("read closed\n");
	}
	System_flush();
    //return ((rxBuffer[2]) + (rxBuffer[1] << 8) + (rxBuffer[0] << 16));
    return ((rxBuffer[0] << 16) + (rxBuffer[1] <<8) + rxBuffer[2]);

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
	else {
		//if(verbose_sensors)System_printf("I2C Initialized!\n");
	}
	System_flush();

    I2C_transfer(handle, &i2cTransaction);

    if(verbose_i2c)System_printf("rxBuffer: 0x%x read from 0x%x\n",rxBuffer[0],address);
    System_flush();

    I2C_close(handle);

	if(verbose_i2c) System_printf("read closed\n");
	System_flush();
    return rxBuffer[0];

}

uint8_t readI2CRegister100kHz(uint8_t board_address, uint8_t address){
	uint8_t			txBuffer[1] = {address};
	uint8_t			rxBuffer[1];

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_100kHz;

    i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 1;
	i2cTransaction.slaveAddress = board_address;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}
	else {
		//if(verbose_sensors)System_printf("I2C Initialized!\n");
	}
	System_flush();

    I2C_transfer(handle, &i2cTransaction);

    if(verbose_i2c)System_printf("rxBuffer: 0x%x read from 0x%x\n",rxBuffer[0],address);
    System_flush();

    I2C_close(handle);

	if(verbose_i2c) System_printf("read closed\n");
	System_flush();
    return rxBuffer[0];
}

uint8_t readEEPROMaddress(uint8_t slaveaddr, uint8_t addrHigh, uint8_t addrLow) {
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
	while (I2C_transfer(handle, &i2cTransaction) == NULL);

	I2C_close(handle);

	return rxBuffer[0];
}
