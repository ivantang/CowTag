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
#include <alphaRadioTest.h>
#include <radioProtocol.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>

/***** Drivers *****/
#include <ti/drivers/PIN.h>
#include <RadioReceive.h>
#include <RadioSend.h>
#include <sensors.h>

/* Board Header files */
#include <Board.h>
#include <pinTable.h>

/***** Defines *****/
#define ALPHARADIOTEST_TASK_STACK_SIZE 1024
#define ALPHARADIOTEST_TASK_PRIORITY   3
#define ALPHARADIOTEST_EVENT_ALL                         0xFFFFFFFF
#define ALPHARADIOTEST_EVENT_NEW_SENSOR_VALUE    (uint32_t)(1 << 0)
#define ALPHARADIOTEST_MAX_NODES 7
#define ALPHARADIOTEST_DISPLAY_LINES 8

/***** Variable declarations *****/
static Task_Params alphaRadioTestTaskParams;
Task_Struct alphaRadioTestTask;    /* not static so you can see in ROV */
static uint8_t alphaRadioTestTaskStack[ALPHARADIOTEST_TASK_STACK_SIZE];
Event_Struct alphaRadioTestEvent;  /* not static so you can see in ROV */
static Event_Handle alphaRadioTestEventHandle;
static struct sensorPacket latestActivePacket;

/***** Prototypes *****/
static void alphaRadioTestTaskFunction(UArg arg0, UArg arg1);
static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi);
void printSampleData(struct sampleData sampledata);
void sendToGateway(struct sampleData sampledata);

/***** Function definitions *****/
void alphaRadioTest_init(void) {

	/* Create event used internally for state changes */
	Event_Params eventParam;
	Event_Params_init(&eventParam);
	Event_construct(&alphaRadioTestEvent, &eventParam);
	alphaRadioTestEventHandle = Event_handle(&alphaRadioTestEvent);

	/* Create the alphaRadioTest radio protocol task */
	Task_Params_init(&alphaRadioTestTaskParams);
	alphaRadioTestTaskParams.stackSize = ALPHARADIOTEST_TASK_STACK_SIZE;
	alphaRadioTestTaskParams.priority = ALPHARADIOTEST_TASK_PRIORITY;
	alphaRadioTestTaskParams.stack = &alphaRadioTestTaskStack;
	Task_construct(&alphaRadioTestTask, alphaRadioTestTaskFunction, &alphaRadioTestTaskParams, NULL);
}

static void alphaRadioTestTaskFunction(UArg arg0, UArg arg1){
	/* Register a packet received callback with the radio task */
	ConcentratorRadioTask_registerPacketReceivedCallback(packetReceivedCallback);

	int delay = 10000;
	struct sampleData sampledata;
	enum NodeRadioOperationStatus results;

	if(verbose_alphaRadioTest){System_printf("Initializing alphaRadioTest...\n");}

	/* Enter main task loop */
	while(1) {
		/* Wait for event */

		/*uint32_t events = Event_pend(alphaRadioTestEventHandle, 0, ALPHARADIOTEST_EVENT_ALL, BIOS_WAIT_FOREVER);

		if(events & ALPHARADIOTEST_EVENT_NEW_SENSOR_VALUE) {
			if(verbose_alphaRadioTest){System_printf("RECEIVED A PACKET\n");}
			if(verbose_alphaRadioTest){printSampleData(latestActivePacket.sampledata);}
		}
*/
		CPUdelay(delay*5000);

		sampledata.cowID = 2;
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

		CPUdelay(delay*5000);

		sampledata.cowID = 2;
		sampledata.packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
		sampledata.timestamp = 0x12345678;

		if(ignoreSensors){
			if(verbose_betaRadioTest){System_printf("Ignoring sensors, making fake packets\n");System_flush();}
			sampledata.accelerometerData.x=0x12;
			sampledata.accelerometerData.y=0x34;
			sampledata.accelerometerData.z=0x56;
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
	}
}

static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi){
		latestActivePacket.header = packet->header;
		latestActivePacket.sampledata = packet->sensorPacket.sampledata;

		Event_post(alphaRadioTestEventHandle, ALPHARADIOTEST_EVENT_NEW_SENSOR_VALUE);
}

/*print what you received*/
void sendToGateway(struct sampleData sampledata){
	//send to Gateway now
//
//	enum NodeRadioOperationStatus results = betaRadioSendData(latestActiveSensorNode.sample);
//
//	// catch a timeout
//	if (results == NodeRadioStatus_Failed) {
//		if(verbose_alphaRadioTestRadioTest){System_printf("Error: %x @ packet: %d\n", latestActiveSensorNode.sample.error, receivedPackets);}
//	}
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
