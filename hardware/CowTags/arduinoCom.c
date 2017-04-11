/*
 * Arduino.c
 *
 *  Created on: Feb 7, 2017
 *      Author: ivan
 */

/***** Includes *****/
#include "global_cfg.h"
#include <radioProtocol.h>
#include <arduinoCom.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* Example/Board Header files */
#include <Board.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/I2C.h> //i2c
#include <ti/drivers/UART.h> //i2c
#include <IIC.h>

#include <stdint.h>
#include <assert.h>

/***** Defines *****/
#define ARDUINO_ADDR	0x08

/***** function definition *****/
/**
 * write an array to a slave i2c device
 * the first element is the slave address
 *
 * @bytes[0]   slave address
 * @bytes[1..] data
 */
void writeI2CArduino(uint8_t slaveAddr, uint8_t bytes[]) {
	if(verbose_arduinoCom){System_printf("Starting arduinoCom\n");}
	uint8_t txBuffer[SAMPLE_SIZE];
	uint8_t rxBuffer[1];

	I2C_Transaction t_i2cTransaction;
	I2C_Handle t_handle;
	I2C_Params t_params;

	// init clock speed and synch
	I2C_Params_init(&t_params);

    t_params.transferMode = I2C_MODE_BLOCKING;
    t_params.bitRate = I2C_100kHz;

    assert(sizeof(bytes) > 0);

    // load data
    int i;
    int arrayLen = SAMPLE_SIZE;
    for (i = 0; i < arrayLen; i++){
    	txBuffer[i] = bytes[i];
    }

    t_i2cTransaction.writeBuf = txBuffer;
    t_i2cTransaction.writeCount = SAMPLE_SIZE;
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = slaveAddr;

    t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		if(verbose_i2c) System_printf("I2C Initialized for Transmitting!\n");
	}

	i = 0;
	for(i = 0; i < retryI2CCount ; i++){
		if(I2C_transfer(t_handle, &t_i2cTransaction) != NULL) break;
		if(verbose_i2c && i == retryI2CCount-1){
			System_printf("I2C Transfer timed out\n");
			System_flush();
		}
	}

	if(verbose_arduinoCom){System_printf("Closing I2C\n");}
	I2C_close(t_handle);
}
