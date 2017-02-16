/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***** Includes *****/
#include <debug.h>
#include <radioProtocol.h>
#include <betaRadioTest.h>

/* XDCtools Header files */
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/knl/Task.h>

/* Drivers */
#include <ti/drivers/PIN.h>
#include <RadioSend.h>
#include <sensors.h>
#include <eeprom.h>

/* Board Header files */
#include <Board.h>
#include <pinTable.h>

/***** Defines *****/
#define BETARADIOTEST_TASK_STACK_SIZE 1024
#define BETARADIOTEST_TASK_PRIORITY   3

/***** Variable Declarations *****/
static Task_Params betaRadioTestTaskParams;
Task_Struct betaRadioTestTask;    /* not static so you can see in ROV */
static uint8_t betaRadioTestTaskStack[BETARADIOTEST_TASK_STACK_SIZE];

/***** Prototypes *****/
static void betaRadioTestTaskFunction(UArg arg0, UArg arg1);
void printSampleData(struct sampleData sampleData);

/***** Function Definitions *****/
void betaRadioTest_init(void)
{
	/* Create the betaRadioTest task */
	Task_Params_init(&betaRadioTestTaskParams);
	betaRadioTestTaskParams.stackSize = BETARADIOTEST_TASK_STACK_SIZE;
	betaRadioTestTaskParams.priority = BETARADIOTEST_TASK_PRIORITY;
	betaRadioTestTaskParams.stack = &betaRadioTestTaskStack;
	Task_construct(&betaRadioTestTask, betaRadioTestTaskFunction, &betaRadioTestTaskParams, NULL);
}

static void betaRadioTestTaskFunction(UArg arg0, UArg arg1)
{
	if(verbose_betaRadioTest){System_printf("Initializing betaRadioTest...\n");}
	struct sampleData sample;
	int receivedPackets = 1;
	sample.cowID = 1;
	sample.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	//sample.timestamp = 0x12345678;
	do {
		if(sample.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){
			System_printf("Creating Packet...\n");
			System_flush();
			makeSensorPacket(&sample);
			System_printf("Packet Created\n");
			System_flush();
			sample.packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
		}
		else{
	//		sample.accelerometerData.x = 0x78;
	//		sample.accelerometerData.y = 0x89;
	//		sample.accelerometerData.z = 0x90;
			makeSensorPacket(&sample);
			sample.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
		}
		sample.error = 0;

		eeprom_reset();

	// 10
//
//	int i;
//	for (i = 0; i < 31; ++i) {
//		eeprom_write(&sample);
//	}
//
//	struct sampleData sample2;
//
//	bool none = eeprom_getNext(&sample2);
	//	if (!none) {

				int delay = 10000;
				CPUdelay(delay*1000);
				enum NodeRadioOperationStatus results = betaRadioSendData(sample);

				// catch a timeout
				if (results == NodeRadioStatus_Failed) {
					if(verbose_betaRadioTest){System_printf("Error: %x @ packet: %d\n", sample.error, receivedPackets);}
	//				break;
				}

				/* Toggle activity LED */
				PIN_setOutputValue(ledPinHandle, BETARADIOTEST_ACTIVITY_LED, !PIN_getOutputValue(BETARADIOTEST_ACTIVITY_LED) );
				++receivedPackets;
	} while (1); //(!eeprom_getNext(&sample2));
	//}

	if(verbose_betaRadioTest){System_printf("DONE: %d\n", receivedPackets);}
	if(verbose_betaRadioTest){System_printf("finished betaRadioTest...\n");}
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
