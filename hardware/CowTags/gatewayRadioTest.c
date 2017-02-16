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
#include <gatewayRadioTest.h>
#include <radioProtocol.h>
#include <serialize.h>
#include <arduinoCom.h>

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
static Task_Params gatewayTaskParams;
Task_Struct gatewayTask;    /* not static so you can see in ROV */
static uint8_t gatewayTaskStack[GATEWAYRADIOTEST_TASK_STACK_SIZE];
Event_Struct gatewayEvent;  /* not static so you can see in ROV */
static Event_Handle gatewayEventHandle;
static struct sensorPacket latestActivePacket;

/***** Prototypes *****/
static void gatewayTaskFunction(UArg arg0, UArg arg1);
static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi);
static void printSampleData(struct sampleData sampledata);

/***** Function definitions *****/
void gatewayRadioTest_init(void) {

	/* Create event used internally for state changes */
	Event_Params eventParam;
	Event_Params_init(&eventParam);
	Event_construct(&gatewayEvent, &eventParam);
	gatewayEventHandle = Event_handle(&gatewayEvent);

	/* Create the gateway radio protocol task */
	Task_Params_init(&gatewayTaskParams);
	gatewayTaskParams.stackSize = GATEWAYRADIOTEST_TASK_STACK_SIZE;
	gatewayTaskParams.priority = GATEWAYRADIOTEST_TASK_PRIORITY;
	gatewayTaskParams.stack = &gatewayTaskStack;
	Task_construct(&gatewayTask, gatewayTaskFunction, &gatewayTaskParams, NULL);
}

static void gatewayTaskFunction(UArg arg0, UArg arg1)
{
	/* Register a packet received callback with the radio task */
	ConcentratorRadioTask_registerPacketReceivedCallback(packetReceivedCallback);

	if(verbose_gatewayRadioTest){System_printf("Initializing gatewayRadioTest...\n");}

	/* Enter main task loop */
	while(1) {
		/* Wait for event */
		uint32_t events = Event_pend(gatewayEventHandle, 0, GATEWAYRADIOTEST_EVENT_ALL, BIOS_WAIT_FOREVER);

		/* If we got a new sensor value */
		if(events & GATEWAYRADIOTEST_EVENT_NEW_SENSOR_VALUE) {

			uint8_t buf[SAMPLE_SIZE];
			unsigned i;

			if(verbose_gatewayRadioTest){printSampleData(latestActivePacket.sampledata);}

			if(verbose_gatewayRadioTest){System_printf("serializing packet...\n");}
			serializePacket(&latestActivePacket.sampledata, buf);

			if(verbose_gatewayRadioTest){
				for(i = 0 ; i<SAMPLE_SIZE; i++){
					System_printf("%i ", buf[i]);
				}
				System_printf("\n");
				System_flush();
			}

			if(verbose_gatewayRadioTest){System_printf("sending packet to ethernet shield...\n");}
			writeI2CArduino(0x6, buf);
		}
	}
}

void printSampleData(struct sampleData sampledata){
	System_printf("gatewayRadio: received packet with CowID = %i, "
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


static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi)
{
	/* Save the values */
	latestActivePacket.header = packet->header;
	latestActivePacket.sampledata = packet->sensorPacket.sampledata;

	Event_post(gatewayEventHandle, GATEWAYRADIOTEST_EVENT_NEW_SENSOR_VALUE);
}

