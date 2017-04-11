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
#include <gatewayRadioTest.h>
#include <radioProtocol.h>
#include <serialize.h>
#include <arduinoCom.h>
#include <stdio.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>

/***** Drivers *****/
#include <RadioReceive.h>

/***** Defines *****/
#define GATEWAYRADIOTEST_TASK_STACK_SIZE 1024
#define GATEWAYRADIOTEST_TASK_PRIORITY   3
#define GATEWAYRADIOTEST_EVENT_ALL                         0xFFFFFFFF
#define GATEWAYRADIOTEST_EVENT_NEW_SENSOR_VALUE    (uint32_t)(1 << 0)

/***** Variable declarations *****/
static Task_Params gatewayRadioTestTaskParams;
Task_Struct gatewayRadioTestTask;    /* not static so you can see in ROV */
static uint8_t gatewayRadioTestTaskStack[GATEWAYRADIOTEST_TASK_STACK_SIZE];
Event_Struct gatewayRadioTestEvent;  /* not static so you can see in ROV */
static Event_Handle gatewayRadioTestEventHandle;
static struct sensorPacket latestActivePacket;

/***** Prototypes *****/
static void gatewayRadioTestTaskFunction(UArg arg0, UArg arg1);
static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi);
static void printSampleData(struct sampleData sampledata);
static void filePrintSampleData(struct sampleData sampledata);

/***** Function definitions *****/
void gatewayRadioTest_init(void) {

	/* Create event used internally for state changes */
	Event_Params eventParam;
	Event_Params_init(&eventParam);
	Event_construct(&gatewayRadioTestEvent, &eventParam);
	gatewayRadioTestEventHandle = Event_handle(&gatewayRadioTestEvent);

	/* Create the gateway radio protocol task */
	Task_Params_init(&gatewayRadioTestTaskParams);
	gatewayRadioTestTaskParams.stackSize = GATEWAYRADIOTEST_TASK_STACK_SIZE;
	gatewayRadioTestTaskParams.priority = GATEWAYRADIOTEST_TASK_PRIORITY;
	gatewayRadioTestTaskParams.stack = &gatewayRadioTestTaskStack;
	Task_construct(&gatewayRadioTestTask, gatewayRadioTestTaskFunction, &gatewayRadioTestTaskParams, NULL);
}

static void gatewayRadioTestTaskFunction(UArg arg0, UArg arg1)
{
	/* Register a packet received callback with the radio task */
	ConcentratorRadioTask_registerPacketReceivedCallback(packetReceivedCallback);

	if(verbose_gatewayRadioTest){System_printf("Initializing gatewayRadioTest...\n");System_flush();}

	/* Enter main task loop */
	while(1) {
		/* Wait for event */
		uint32_t events = Event_pend(gatewayRadioTestEventHandle, 0, GATEWAYRADIOTEST_EVENT_ALL, BIOS_WAIT_FOREVER);

		/* If we got a new sensor value */
		if(events & GATEWAYRADIOTEST_EVENT_NEW_SENSOR_VALUE) {

			uint8_t buf[SAMPLE_SIZE];
			unsigned i;

			// show results
			printSampleData(latestActivePacket.sampledata);

			if(verbose_gatewayRadioTest){System_printf("serializing packet...\n");System_flush();}
			serializePacket(&latestActivePacket.sampledata, buf);

			if(verbose_gatewayRadioTest){
				for(i = 0 ; i<SAMPLE_SIZE; i++){
					System_printf("%i ", buf[i]);
				}
				System_printf("\n");
				System_flush();
			}

			if(verbose_gatewayRadioTest){System_printf("sending packet to ethernet shield...\n");System_flush();}
			writeI2CArduino(0x6, buf);
			if(verbose_gatewayRadioTest){System_printf("packet sent to ethernet shield...\n");System_flush();}
		}
	}
}

// display incoming samples in STDOUT
void printSampleData(struct sampleData sampledata){
	System_printf("BetaTest: sent packet with CowID = %i, "
		"PacketType: %i, "
		"Timestamp: %i, "
		"Error: %i, ",
		sampledata.cowID,
		sampledata.packetType,
		sampledata.timestamp,
		sampledata.error);
	if(sampledata.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){
		System_printf(							"TemperatureCowData = %i, "
			"AmbientTemperatureData = %i, "
			"InfraredData = %i\n",
			sampledata.tempData.temp_h << 8 |
			sampledata.tempData.temp_l,
			sampledata.heartRateData.temp_h << 8 |
			sampledata.heartRateData.temp_l,
			sampledata.heartRateData.rate_l);
	}
	else{
		System_printf(							"accelerometerData= x=%i, y=%i, z=%i\n",
			sampledata.accelerometerData.x_h << 8 + sampledata.accelerometerData.x_l,
			sampledata.accelerometerData.y_h << 8 + sampledata.accelerometerData.y_l,
			sampledata.accelerometerData.z_h << 8 + sampledata.accelerometerData.z_l);
	}
}

// write samples to a file instead of STDOUT
void filePrintSampleData(struct sampleData sampledata) {
	static bool file_is_initialized = false;
	static FILE *fp;

	if (!file_is_initialized) {
		fp = fopen("../gateway_packet_output.txt", "w");
		file_is_initialized = true;
	}


	fprintf(fp, "GatewayRadio: received packet with CowID = %i, PacketType: %i, "
			"Timestamp: %i, Error: %i, ",
			sampledata.cowID,
			sampledata.packetType,
			sampledata.timestamp,
			sampledata.error);
	if(sampledata.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){
		fprintf(fp, "TemperatureCowData = %i.%i, "
				"AmbientTemperatureData = %i.%i, "
				"HeartRate = %i\n",
				sampledata.tempData.temp_h,
				sampledata.tempData.temp_l,
				sampledata.heartRateData.temp_h,
				sampledata.heartRateData.temp_l,
				sampledata.heartRateData.rate_h,
				sampledata.heartRateData.rate_l);
	}
	else{
		fprintf(fp, "accelerometerData= x=%i, y=%i, z=%i\n",
				sampledata.accelerometerData.x_h << 8 + sampledata.accelerometerData.x_l,
				sampledata.accelerometerData.y_h << 8 + sampledata.accelerometerData.y_l,
				sampledata.accelerometerData.z_h << 8 + sampledata.accelerometerData.z_l);
	}
}


static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi)
{
	/* Save the values */
	latestActivePacket.header = packet->header;
	latestActivePacket.sampledata = packet->sensorPacket.sampledata;

	Event_post(gatewayRadioTestEventHandle, GATEWAYRADIOTEST_EVENT_NEW_SENSOR_VALUE);
}

