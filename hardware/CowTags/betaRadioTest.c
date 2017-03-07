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
#include <ti/sysbios/knl/Event.h>

/* Drivers */
#include <ti/drivers/PIN.h>
#include <RadioSend.h>
#include <sensors.h>
#include <eeprom.h>
#include <Sleep.h>
#include <EventManager.h>

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
static Event_Handle * eventHandle;

/***** Prototypes *****/
static void betaRadioTestTaskFunction(UArg arg0, UArg arg1);
void printSampleData(struct sampleData sampleData);

/***** Function Definitions *****/
void betaRadioTest_init(void)
{
	/* Get master event handle for internal state changes */
	eventHandle = getEventHandle();

	/* Create the betaRadioTest task */
	Task_Params_init(&betaRadioTestTaskParams);
	betaRadioTestTaskParams.stackSize = BETARADIOTEST_TASK_STACK_SIZE;
	betaRadioTestTaskParams.priority = BETARADIOTEST_TASK_PRIORITY;
	betaRadioTestTaskParams.stack = &betaRadioTestTaskStack;
	Task_construct(&betaRadioTestTask, betaRadioTestTaskFunction, &betaRadioTestTaskParams, NULL);
}

static void betaRadioTestTaskFunction(UArg arg0, UArg arg1)
{
	if(verbose_betaRadioTest){System_printf("Initializing betaRadioTest...\n");System_flush();}

	while(1){
		int delay = 10000;
		struct sampleData sampledata;
		enum NodeRadioOperationStatus results;

		sampledata.cowID = 1;
		sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
		sampledata.timestamp = 0x12345678;

		if(ignoreSensors){
			if(verbose_betaRadioTest){System_printf("Ignoring sensors, making fake packets\n");System_flush();}
			sampledata.tempData.temp_h = 0x78;
			sampledata.tempData.temp_l = 0x65;
			sampledata.heartRateData.rate_h = 0x90;
			sampledata.heartRateData.rate_l = 0x87;
			sampledata.heartRateData.temp_h = 0x45;
			sampledata.heartRateData.temp_l = 0x32;
			sampledata.error = 0x0;
		} else {
			if(verbose_betaRadioTest){System_printf("Creating Packet...\n");System_flush();}
			makeSensorPacket(&sampledata);
			if(verbose_betaRadioTest){System_printf("Packet Created\n");System_flush();}
		}

		if(verbose_betaRadioTest){printSampleData(sampledata);}
		if(verbose_betaRadioTest){System_printf("sending packet...\n");System_flush();}
		results = betaRadioSendData(sampledata);
		if(verbose_betaRadioTest){System_printf("packet sent error: %i\n",results);System_flush();}
//
//		CPUdelay(delay*5000);
//
//		sampledata.cowID = 1;
//		sampledata.packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
//		sampledata.timestamp = 0x12345678;
//
//		if(ignoreSensors){
//			if(verbose_betaRadioTest){System_printf("Ignoring sensors, making fake packets\n");System_flush();}
//			sampledata.accelerometerData.x=0x12;
//			sampledata.accelerometerData.y=0x34;
//			sampledata.accelerometerData.z=0x56;
//			sampledata.error = 0x0;
//		} else {
//			if(verbose_betaRadioTest){System_printf("Creating Packet...\n");System_flush();}
//			makeSensorPacket(&sampledata);
//			if(verbose_betaRadioTest){System_printf("Packet Created\n");System_flush();}
//		}
//
//		if(verbose_betaRadioTest){printSampleData(sampledata);}
//		if(verbose_betaRadioTest){System_printf("sending packet...\n");System_flush();}
//		results = betaRadioSendData(sampledata);
//		if(verbose_betaRadioTest){System_printf("packet sent error: %i\n",results);System_flush();}
//		Task_Handle *tsk = getTaskHandle(TASK_RADIO_SEND);

		System_printf("betaRadioTest going to sleep!\n");
		Task_sleep(sleepAMinute());
		System_printf("betaRadioTest waking up :)\n");
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
											"InfraredData = %i.%i\n",
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
