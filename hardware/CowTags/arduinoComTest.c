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

/* Board Header files */
#include <Board.h>

/***** Defines *****/
#define TASKSTACKSIZE		1024

/***** Variable Declarations *****/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];
struct sampleData sampledata;

/***** Prototypes *****/
void arduinoComTest(void);
void printSampleData(struct sampleData sampleData);

/***** Function Definitions *****/
void arduinoComTest_init(void){
	Task_Params taskParams;

	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)arduinoComTest, &taskParams, NULL);
}

void arduinoComTest(void){
	while(1){
	int delay = 10000;
	CPUdelay(delay*5000);
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
	sampledata.error = 0x0;
	if(verbose_arduinoComTest){printSampleData(sampledata);}

	serializePacket(&sampledata, buf);

	if(verbose_arduinoComTest){
		for(i = 0 ; i<SAMPLE_SIZE; i++){
			System_printf("%i ", buf[i]);
		}
		System_printf("\n");
		System_flush();
	}

	writeI2CArduino(0x6, buf);

	CPUdelay(delay*5000);

	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.accelerometerData.x=0x12;
	sampledata.accelerometerData.y=0x34;
	sampledata.accelerometerData.z=0x56;
	sampledata.error = 0x0;
	if(verbose_arduinoComTest){printSampleData(sampledata);}

	serializePacket(&sampledata, buf);

	if(verbose_arduinoComTest){
		for(i = 0 ; i<SAMPLE_SIZE; i++){
			System_printf("%i ", buf[i]);
		}
		System_printf("\n");
		System_flush();
	}

	writeI2CArduino(0x6, buf);

	if(verbose_arduinoComTest){System_printf("testArduino finished\n");}
	System_flush();
	}
}

void printSampleData(struct sampleData sampledata){
	System_printf("BetaRadio: sent packet with CowID = %i, "
											"PacketType: %i, "
											"Timestamp: %i, "
											"Error: %i, ",
											sampledata.cowID,
											sampledata.packetType,
											sampledata.timestamp,
											sampledata.error);
	if(sampledata.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){
	System_printf(							"TemperatureCowData = %i.%i, "
											"AmbientTemperatureData = %i.%i, "
											"InfraredData = %i.%i\n ",
											sampledata.tempData.temp_h,
											sampledata.tempData.temp_l,
											sampledata.heartRateData.temp_h,
											sampledata.heartRateData.temp_l,
											sampledata.heartRateData.rate_h,
											sampledata.heartRateData.rate_l);
	}
	else{
	System_printf(							"accelerometerData= x=%i, y=%i, z=%i\n",
											sampledata.accelerometerData.x,
											sampledata.accelerometerData.y,
											sampledata.accelerometerData.z);
	}
}
