/*
 * alphaSendReceiveTest.c
 *
 *  Created on: Mar 2, 2017
 *      Author: annik
 */

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
#include "global_cfg.h"
#include <alphaRadioTest.h>
#include <radioProtocol.h>
#include <stdio.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Timestamp.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>

/***** Drivers *****/
#include <ti/drivers/PIN.h>
#include <radioSendReceive.h>
#include <sensors.h>

/* Board Header files */
#include <Board.h>
#include <pinTable.h>
#include <Sleep.h>
#include <EventManager.h>


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
static Event_Handle *alphaRadioTestEventHandle;
static struct sensorPacket latestActivePacket;
static int received = 0;

/***** Prototypes *****/
static void alphaRadioTestTaskFunction(UArg arg0, UArg arg1);
static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi);
void printSampleData(struct sampleData sampledata);
void file_printSampleData(struct sampleData sampledata);
void sendToGateway(struct sampleData sampledata);

/***** Function definitions *****/
void alphaRadioTest_init(void) {

	/* Create event used internally for state changes */
	alphaRadioTestEventHandle = getEventHandle();

	/* Create the alphaRadioTest radio protocol task */
	Task_Params_init(&alphaRadioTestTaskParams);
	alphaRadioTestTaskParams.stackSize = ALPHARADIOTEST_TASK_STACK_SIZE;
	alphaRadioTestTaskParams.priority = ALPHARADIOTEST_TASK_PRIORITY;
	alphaRadioTestTaskParams.stack = &alphaRadioTestTaskStack;
	Task_construct(&alphaRadioTestTask, alphaRadioTestTaskFunction, &alphaRadioTestTaskParams, NULL);
}

static void alphaRadioTestTaskFunction(UArg arg0, UArg arg1){

	struct sampleData sampledata;
	enum alphaRadioOperationStatus results;
	if(verbose_alphaRadioTest){System_printf("Initializing alphaRadioTest...\n");}

	while (1) {
		received = 0;
		Task_sleep(2 * sleepASecond());
		// -------------------- SENDING -------------------------
		sampledata.cowID = 2;
		sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
		sampledata.timestamp = 0x12345678;

		// make/fake a sensor packet
		if(ignoreSensors){
			if(verbose_alphaRadioTest){System_printf("SEND: Ignoring sensors, making fake packets\n");System_flush();}
			sampledata.tempData.temp_h = 0x78;
			sampledata.tempData.temp_l = 0x65;
			sampledata.heartRateData.rate_h = 0x90;
			sampledata.heartRateData.rate_l = 0x87;
			sampledata.heartRateData.temp_h = 0x45;
			sampledata.heartRateData.temp_l = 0x32;
			sampledata.error = 0x0;
			getTimestamp(&sampledata);
		} else {
			if(verbose_alphaRadioTest){System_printf("SEND: Creating Packet...");System_flush();}
			makeSensorPacket(&sampledata);
		}

		// send packet
		if(verbose_alphaRadioTest){System_printf("SEND: sending packet...\n");System_flush();}
		results = alphaRadioSendData(sampledata);
		if (results == AlphaRadioStatus_Success){
			if(verbose_alphaRadioTest){System_printf("SEND: Sent Correctly!\n");}
		}else{
			if(verbose_alphaRadioTest){System_printf("SEND: packet sent error: %i\n",results);System_flush();}
		}

		// NOTE:
		// It's not in our requirements to have asynchronous send and receive
		// rather, we wish to send and receive at different times.
		// Although it is necessary for the radioSendReceive thread to be able to do both!

		Task_sleep(2 * sleepASecond());

		AlphaRadioTask_registerPacketReceivedCallback(packetReceivedCallback); // register callback
		while(Timestamp_get32()%2 != 0);
		results = alphaRadioReceiveData();	// start listening, obtain radioAccessSem
		if (results == AlphaRadioStatus_ReceivedValidPacket||
				received == 1) {
			if(verbose_alphaRadioTest){
				System_printf("RECEIVE: received a packet.\n");
				printSampleData(latestActivePacket.sampledata);
			}
		}  else{
			if(verbose_alphaRadioTest){
				System_printf("RECEIVE: did not receive packet.\n");System_flush();}
		}

	}
}

/*callback for received sensor packets*/
void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi) {
	latestActivePacket.header = packet->header;
	latestActivePacket.sampledata = packet->sensorPacket.sampledata;
	//printSampleData(latestActivePacket.sampledata);
	received = 1;
}

/*print the received packet*/
void printSampleData(struct sampleData sampledata){
	System_printf("ALPHA: received packet with CowID = %i, "
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

void file_printSampleData(struct sampleData sampledata) {
	FILE *fp;

	fp = fopen("../alpha_packet_output.txt", "a");

	fprintf(fp, "BetaRadio: sent packet with CowID = %i, PacketType: %i, "
			"Timestamp: %i, Error: %i, ",
			sampledata.cowID,
			sampledata.packetType,
			sampledata.timestamp,
			sampledata.error);
	if(sampledata.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){
		fprintf(fp, "TemperatureCowData = %i.%i, "
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
		fprintf(fp, "accelerometerData= x=%i, y=%i, z=%i\n",
				sampledata.accelerometerData.x,
				sampledata.accelerometerData.y,
				sampledata.accelerometerData.z);
	}
	fclose(fp);
}



