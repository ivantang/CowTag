/*
 * ArduinoCom.h
 *
 *  Created on: Feb 10, 2017
 *      Author: Erik-PC
 */

/***** Includes *****/
#include <debug.h>
#include <radioProtocol.h>
#include <arduinoComTest.h>
#include <arduinoCom.h>
#include <serialize.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/***** Defines *****/
#define TASKSTACKSIZE		1024

/***** Variable Declarations *****/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];
struct sampleData sampledata;

/***** Prototypes *****/
void arduinoComTest(void);

/***** Function Definitions *****/
void arduinoComTest_init(void){
	Task_Params taskParams;

	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)arduinoComTest, &taskParams, NULL);
}

void arduinoComTest(void){
	if(verbose_arduinoComTest){System_printf("Starting arduinoComTest\n");}

	uint8_t buf[SAMPLE_SIZE];
	unsigned i;

	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.tempData.temp_h = 0x78;
	sampledata.tempData.temp_l = 0x65;
	sampledata.heartRateData.rate_h = 0x90;
	sampledata.heartRateData.rate_l = 0x87;
	sampledata.heartRateData.temp_h = 0x45;
	sampledata.heartRateData.temp_l = 0x32;

	serializePacket(&sampledata, buf);

	if(verbose_arduinoComTest){
		for(i = 0 ; i<SAMPLE_SIZE; i++){
			System_printf("%x ", buf[i]);
		}
		System_printf("\n");
		System_flush();
	}

	writeI2CArduino(0x6, buf);

	if(verbose_arduinoComTest){System_printf("testArduino finished\n");}
	System_flush();
}
