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

#include <alphaRadioTest.h>
#include <radioProtocol.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>

/* Drivers */
#include <ti/drivers/PIN.h>
#include <ti/mw/display/Display.h>
#include <ti/mw/display/DisplayExt.h>

/* Board Header files */
#include "Board.h"
#include "RadioReceive.h"
#include "radioProtocol.h"
#include "debug.h"


/***** Defines *****/
#define CONCENTRATOR_TASK_STACK_SIZE 1024
#define CONCENTRATOR_TASK_PRIORITY   3
#define CONCENTRATOR_EVENT_ALL                         0xFFFFFFFF
#define CONCENTRATOR_EVENT_NEW_SENSOR_VALUE    (uint32_t)(1 << 0)
#define CONCENTRATOR_MAX_NODES 7
#define CONCENTRATOR_DISPLAY_LINES 8

/***** Type declarations *****/
struct BetaSensorNode {
	uint8_t valid;
	uint8_t address;
	struct sampleData sampledata;
	int8_t latestRssi;
};


/***** Variable declarations *****/
static Task_Params concentratorTaskParams;
Task_Struct concentratorTask;    /* not static so you can see in ROV */
static uint8_t concentratorTaskStack[CONCENTRATOR_TASK_STACK_SIZE];
Event_Struct concentratorEvent;  /* not static so you can see in ROV */
static Event_Handle concentratorEventHandle;
static struct sensorPacket latestActiveSensorNode;
//struct BetaSensorNode knownSensorNodes[CONCENTRATOR_MAX_NODES];
//static struct BetaSensorNode* lastAddedSensorNode = knownSensorNodes;
static int i = 0;


/***** Prototypes *****/
static void concentratorTaskFunction(UArg arg0, UArg arg1);
static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi);
static void addNewNode(struct BetaSensorNode* node);
static void updateNode(struct BetaSensorNode* node);
static uint8_t isKnownNodeAddress(uint8_t address);
static void printNodes(void);

/***** Function definitions *****/
void alphaRadioTest_init(void) {

	/* Create event used internally for state changes */
	Event_Params eventParam;
	Event_Params_init(&eventParam);
	Event_construct(&concentratorEvent, &eventParam);
	concentratorEventHandle = Event_handle(&concentratorEvent);

	/* Create the concentrator radio protocol task */
	Task_Params_init(&concentratorTaskParams);
	concentratorTaskParams.stackSize = CONCENTRATOR_TASK_STACK_SIZE;
	concentratorTaskParams.priority = CONCENTRATOR_TASK_PRIORITY;
	concentratorTaskParams.stack = &concentratorTaskStack;
	Task_construct(&concentratorTask, concentratorTaskFunction, &concentratorTaskParams, NULL);
}

static void concentratorTaskFunction(UArg arg0, UArg arg1)
{
	/* Register a packet received callback with the radio task */
	ConcentratorRadioTask_registerPacketReceivedCallback(packetReceivedCallback);

	System_printf("Starting radio test\n");

	/* Enter main task loop */
	while(1) {
		/* Wait for event */
		uint32_t events = Event_pend(concentratorEventHandle, 0, CONCENTRATOR_EVENT_ALL, BIOS_WAIT_FOREVER);

		/* If we got a new sensor value */
		/*if(events & CONCENTRATOR_EVENT_NEW_SENSOR_VALUE) {
			// If we knew this node from before, update the value
			if(isKnownNodeAddress(latestActiveSensorNode.address)) {
				updateNode(&latestActiveSensorNode);
			}
			else {
				// Else add it
				addNewNode(&latestActiveSensorNode);
			}

		}*/

		if(events & CONCENTRATOR_EVENT_NEW_SENSOR_VALUE) {
			i++;
			printNodes();
		}

	}
}

static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi)
{
		/* Save the values */
		//latestActiveSensorNode.valid=1;
		latestActiveSensorNode.header.sourceAddress = packet->header.sourceAddress;
		latestActiveSensorNode.sampledata = packet->sensorPacket.sampledata;
		//latestActiveSensorNode.latestRssi = rssi;

		Event_post(concentratorEventHandle, CONCENTRATOR_EVENT_NEW_SENSOR_VALUE);
}

/*static uint8_t isKnownNodeAddress(uint8_t address) {
	uint8_t found = 0;
	uint8_t i;
	for (i = 0; i < CONCENTRATOR_MAX_NODES; i++)
	{
		if (knownSensorNodes[i].address == address)
		{
			found = 1;
			break;
		}
	}
	return found;
}*/

/*print what you received*/
void printNodes(void) {
		System_printf(	"%d th packet, "
						"received packet from "
						"src address = 0x%x, "
						"Error code: 0x%x\n",
						//"Temp_Data = %i.%i, "
						//"Acc_Data= x=%i y=%i z=%i, "
						//"IR_Data_H = %i, IR_Data_L = %i \n",
						i,
						latestActiveSensorNode.header.sourceAddress,
						latestActiveSensorNode.sampledata.error);
						//latestActiveSensorNode.sampledata.tempData.temp_h,
						//latestActiveSensorNode.sampledata.tempData.temp_l,

						//latestActiveSensorNode.sampledata.accelerometerData.x,
						//latestActiveSensorNode.sampledata.accelerometerData.y,
						//latestActiveSensorNode.sampledata.accelerometerData.z,

						//latestActiveSensorNode.sampledata.heartRateData.rate_h,
						//latestActiveSensorNode.sampledata.heartRateData.rate_l);
}

/*static void updateNode(struct BetaSensorNode* node) {
	uint8_t i;
	for (i = 0; i < CONCENTRATOR_MAX_NODES; i++) {
		if (knownSensorNodes[i].address == node->address)
		{
			knownSensorNodes[i].sampledata = node->sampledata;
			knownSensorNodes[i].latestRssi = node->latestRssi;
			break;
		}
	}
}*/

/*static void addNewNode(struct BetaSensorNode* node) {
	*lastAddedSensorNode = *node;

	// Increment and wrap
	lastAddedSensorNode++;
	if (lastAddedSensorNode > &knownSensorNodes[CONCENTRATOR_MAX_NODES-1])
	{
		lastAddedSensorNode = knownSensorNodes;
	}
}*/


