/*
 * Arduino.c
 *
 *  Created on: Feb 7, 2017
 *      Author: ivan
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
#include <ti/drivers/I2C.h> //i2c
#include <ti/drivers/UART.h> //i2c

/* Example/Board Header files */
#include "Board.h"
#include <debug.h>
#include <stdint.h>
#include <assert.h>

#include <IIC.c>
#include <IIC.h>
#include <Arduino.h>
#include <eeprom.h>
#include <serialize.h>

/**constants*/
#define TASKSTACKSIZE		1024	//i2c

/**structures*/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*function definition */
void Arduino_init(void){
	Task_Params taskParams;

	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)testArduino, &taskParams, NULL);
}

/**
 * write an array to a slave i2c device
 * the first element is the slave address
 *
 * @bytes[0]   slave address
 * @bytes[1..] data
 */
static void writeI2CArduino(uint8_t slaveAddr, uint8_t bytes[]) {
	uint8_t			txBuffer[SAMPLE_SIZE];
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

	while(I2C_transfer(t_handle, &t_i2cTransaction) == NULL);

	I2C_close(t_handle);
}

void testArduino(){
	//uint8_t Packet[] = {0x1,0x2,0x3,0x4};
	//writeI2CArduino(0x6,Packet);
	uint8_t buf[SAMPLE_SIZE];
	unsigned i;

	struct sampleData sampledata;
	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.tempData.temp_h = 0x78;
	sampledata.tempData.temp_l = 0x65;
	sampledata.heartRateData.rate_h = 0x90;
	sampledata.heartRateData.rate_l = 0x87;
	sampledata.heartRateData.temp_h = 0x45;
	sampledata.heartRateData.temp_l = 0x32;
	sampledata.accelerometerData.x = 0x12;
	sampledata.accelerometerData.y = 0x13;
	sampledata.accelerometerData.z = 0x14;

	while(1){
		serializePacket(&sampledata, buf);

		for(i = 0 ; i<SAMPLE_SIZE; i++){
			System_printf("%x ", buf[i]);
		}
		System_printf("\n");
		System_flush();
		writeI2CArduino(0x6, buf);


		//writeI2CArray(ARDUINO_ADDR,Packet);
		//writeI2CRegister(0x6,0x1,0x2);
		//writeI2CRegister(ARDUINO_ADDR,0x3,0x4);
		System_printf("testArduino finished\n");
		System_flush();

		Task_sleep(1000000);
	}
}
