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
#include "alphaTask.h"
#include "radioProtocol.h"
#include "debug.h"


/***** Defines *****/
#define CONCENTRATOR_TASK_STACK_SIZE 1024
#define CONCENTRATOR_TASK_PRIORITY   3
#define CONCENTRATOR_EVENT_ALL                         0xFFFFFFFF
#define CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE    (uint32_t)(1 << 0)
#define CONCENTRATOR_MAX_NODES 7
#define CONCENTRATOR_DISPLAY_LINES 8

/***** Type declarations *****/
struct AdcSensorNode {
	uint8_t address;
	uint16_t latestAdcValue;
	uint8_t button;
	int8_t latestRssi;
};


/***** Variable declarations *****/
static Task_Params concentratorTaskParams;
Task_Struct concentratorTask;    /* not static so you can see in ROV */
static uint8_t concentratorTaskStack[CONCENTRATOR_TASK_STACK_SIZE];
Event_Struct concentratorEvent;  /* not static so you can see in ROV */
static Event_Handle concentratorEventHandle;
static struct AdcSensorNode latestActiveAdcSensorNode;
struct AdcSensorNode knownSensorNodes[CONCENTRATOR_MAX_NODES];
static struct AdcSensorNode* lastAddedSensorNode = knownSensorNodes;
static Display_Handle hDisplayLcd;
static Display_Handle hDisplaySerial;


/***** Prototypes *****/
static void concentratorTaskFunction(UArg arg0, UArg arg1);
static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi);
static void updateLcd(void);
static void addNewNode(struct AdcSensorNode* node);
static void updateNode(struct AdcSensorNode* node);
static uint8_t isKnownNodeAddress(uint8_t address);


/***** Function definitions *****/
void AlphaTask_init(void) {

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
	/* Initialize display and try to open both UART and LCD types of display. */
	Display_Params params;
	Display_Params_init(&params);
	params.lineClearMode = DISPLAY_CLEAR_BOTH;

	/* Open both an available LCD display and an UART display.
	 * Whether the open call is successful depends on what is present in the
	 * Display_config[] array of the board file.
	 *
	 * Note that for SensorTag evaluation boards combined with the SHARP96x96
	 * Watch DevPack, there is a pin conflict with UART such that one must be
	 * excluded, and UART is preferred by default. To display on the Watch
	 * DevPack, add the precompiler define BOARD_DISPLAY_EXCLUDE_UART.
	 */
	hDisplayLcd = Display_open(Display_Type_LCD, &params);
	hDisplaySerial = Display_open(Display_Type_UART, &params);

	/* Check if the selected Display type was found and successfully opened */
	if (hDisplaySerial)
	{
		Display_print0(hDisplaySerial, 0, 0, "Waiting for nodes...");
	}

	/* Check if the selected Display type was found and successfully opened */
	if (hDisplayLcd)
	{
		Display_print0(hDisplayLcd, 0, 0, "Waiting for nodes...");
	}

	/* Register a packet received callback with the radio task */
	ConcentratorRadioTask_registerPacketReceivedCallback(packetReceivedCallback);

	/* Enter main task loop */
	while(1) {
		/* Wait for event */
		uint32_t events = Event_pend(concentratorEventHandle, 0, CONCENTRATOR_EVENT_ALL, BIOS_WAIT_FOREVER);

		/* If we got a new ADC sensor value */
		if(events & CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE) {
			/* If we knew this node from before, update the value */
			if(isKnownNodeAddress(latestActiveAdcSensorNode.address)) {
				updateNode(&latestActiveAdcSensorNode);
			}
			else {
				/* Else add it */
				addNewNode(&latestActiveAdcSensorNode);
			}

			/* Update the values on the LCD */
			updateLcd();
		}
	}
}

static void packetReceivedCallback(union ConcentratorPacket* packet, int8_t rssi)
{
	/* If we recived an ADC sensor packet, for backward compatibility */
	if (packet->header.packetType == RADIO_PACKET_TYPE_BETA_PACKET)
	{
		/* Save the values */
		latestActiveAdcSensorNode.address = packet->header.sourceAddress;
		latestActiveAdcSensorNode.latestAdcValue = packet->betaPacket.betadata.sensorData;
		latestActiveAdcSensorNode.button = 0; //no button value in ADC packet
		latestActiveAdcSensorNode.latestRssi = rssi;

		Event_post(concentratorEventHandle, CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE);
	}
	/* If we recived an DualMode ADC sensor packet*/
	else if(packet->header.packetType == RADIO_PACKET_TYPE_DM_SENSOR_PACKET)
	{

		/* Save the values */
		latestActiveAdcSensorNode.address = packet->header.sourceAddress;
		latestActiveAdcSensorNode.latestAdcValue = packet->dmSensorPacket.adcValue;
		latestActiveAdcSensorNode.button = packet->dmSensorPacket.button;
		latestActiveAdcSensorNode.latestRssi = rssi;

		Event_post(concentratorEventHandle, CONCENTRATOR_EVENT_NEW_ADC_SENSOR_VALUE);
	}
}

static uint8_t isKnownNodeAddress(uint8_t address) {
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
}

static void updateNode(struct AdcSensorNode* node) {
	uint8_t i;
	for (i = 0; i < CONCENTRATOR_MAX_NODES; i++) {
		if (knownSensorNodes[i].address == node->address)
		{
			knownSensorNodes[i].latestAdcValue = node->latestAdcValue;
			knownSensorNodes[i].latestRssi = node->latestRssi;
			knownSensorNodes[i].button = node->button;
			break;
		}
	}
}

static void addNewNode(struct AdcSensorNode* node) {
	*lastAddedSensorNode = *node;

	/* Increment and wrap */
	lastAddedSensorNode++;
	if (lastAddedSensorNode > &knownSensorNodes[CONCENTRATOR_MAX_NODES-1])
	{
		lastAddedSensorNode = knownSensorNodes;
	}
}

static void updateLcd(void) {
	struct AdcSensorNode* nodePointer = knownSensorNodes;
	uint8_t currentLcdLine;

	/* Clear the display and write header on first line */
	Display_clear(hDisplayLcd);
	Display_print0(hDisplayLcd, 0, 0, "Nodes Value SW  RSSI");

	//clear screen, put cuser to beggining of terminal and print the header
	Display_print0(hDisplaySerial, 0, 0, "\033[2J \033[0;0HNodes   Value   SW    RSSI");

	/* Start on the second line */
	currentLcdLine = 1;

	/* Write one line per node */
	while ((nodePointer < &knownSensorNodes[CONCENTRATOR_MAX_NODES]) &&
			(nodePointer->address != 0) &&
			(currentLcdLine < CONCENTRATOR_DISPLAY_LINES))
	{
		/* print to LCD */
		Display_print4(hDisplayLcd, currentLcdLine, 0, "0x%02x  %04d  %d   %04d",
				nodePointer->address, nodePointer->latestAdcValue, nodePointer->button,
				nodePointer->latestRssi);

		/* print to UART */
		Display_print4(hDisplaySerial, 0, 0, "0x%02x    %04d    %d    %04d",
				nodePointer->address, nodePointer->latestAdcValue, nodePointer->button,
				nodePointer->latestRssi);

		nodePointer++;
		currentLcdLine++;
	}
}
