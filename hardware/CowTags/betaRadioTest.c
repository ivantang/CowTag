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

#include <betaRadioTest.h>
#include <radioProtocol.h>
#include <RadioSend.h>
#include <debug.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Clock.h>

#include <ti/drivers/PIN.h>

/* Board Header files */
#include "Board.h"
#include "debug.h"
#include "pinTable.h"
#include "RadioSend.h"


/***** Defines *****/
#define NODE_TASK_STACK_SIZE 1024
#define NODE_TASK_PRIORITY   3

#define NODE_EVENT_ALL                  0xFFFFFFFF
#define NODE_EVENT_NEW_VALUE    (uint32_t)(1 << 0)


/***** Variable declarations *****/
static Task_Params nodeTaskParams;
Task_Struct nodeTask;    /* not static so you can see in ROV */
static uint8_t nodeTaskStack[NODE_TASK_STACK_SIZE];
Event_Struct nodeEvent;  /* not static so you can see in ROV */
static Event_Handle nodeEventHandle;

/*constants*/
static struct sampleData sampledata;

/***** Prototypes *****/
static void nodeTaskFunction(UArg arg0, UArg arg1);
void fastReportTimeoutCallback(UArg arg0);
void betaCallBack(struct sampleData newsampledata);


/***** Function definitions *****/
void betaRadioTest_init(void)
{
	/* Create event used internally for state changes */
	Event_Params eventParam;
	Event_Params_init(&eventParam);
	Event_construct(&nodeEvent, &eventParam);
	nodeEventHandle = Event_handle(&nodeEvent);

	/* Create the node task */
	Task_Params_init(&nodeTaskParams);
	nodeTaskParams.stackSize = NODE_TASK_STACK_SIZE;
	nodeTaskParams.priority = NODE_TASK_PRIORITY;
	nodeTaskParams.stack = &nodeTaskStack;
	Task_construct(&nodeTask, nodeTaskFunction, &nodeTaskParams, NULL);
}

// write thrice and send thrice!
static void nodeTaskFunction(UArg arg0, UArg arg1)
{
//		sampledata.tempData = getObjTemp();
//		sampledata.accelerometerData = getAcceleration();
//		sampledata.heartRateData = getHeartRate();
//		int delay = 10000;
//		CPUdelay(delay*1000);

	// fake sensor data
	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.tempData.temp_h = 0x5678;
	sampledata.tempData.temp_l = 0x8765;
	sampledata.heartRateData.rate_h = 0x7890;
	sampledata.heartRateData.rate_l = 0x0987;
	sampledata.heartRateData.temp_h = 0x2345;
	sampledata.heartRateData.temp_l = 0x5432;
	sampledata.error = 0;

	eeprom_reset();

	// 10
	int i;
	for (i = 0; i < 62; ++i) {
		eeprom_write(&sampledata);
	}

	struct sampleData sample2;

	bool none = eeprom_getNext(&sample2);
	int receivedPackets = 1;
	if (!none) {
		do {
			enum NodeRadioOperationStatus results = betaRadioSendData(sample2);

			System_printf("Error: %x @ packet: %d\n", sampledata.error, receivedPackets);
			// catch a timeout
			if (results == NodeRadioStatus_Failed) {
				break;
			}

			PIN_setOutputValue(ledPinHandle, NODE_ACTIVITY_LED, !PIN_getOutputValue(NODE_ACTIVITY_LED) );
			++receivedPackets;
		} while (!eeprom_getNext(&sample2));
	}

	System_printf("DONE: %d\n", receivedPackets);
}
