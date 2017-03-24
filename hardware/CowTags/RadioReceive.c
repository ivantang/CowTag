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
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>

/* Board Header files */
#include "Board.h"
#include "easylink/EasyLink.h"
#include "radioProtocol.h"
#include "RadioReceive.h"
#include "pinTable.h"
#include <config_parse.h>

// Config File
#include "global_cfg.h"


/***** Defines *****/
#define CONCENTRATORRADIO_TASK_STACK_SIZE 1024
#define CONCENTRATORRADIO_TASK_PRIORITY   3

#define RADIO_EVENT_ALL                  0xFFFFFFFF
#define RADIO_EVENT_VALID_PACKET_RECEIVED      (uint32_t)(1 << 0)
#define RADIO_EVENT_INVALID_PACKET_RECEIVED (uint32_t)(1 << 1)


/***** Variable declarations *****/
static Task_Params concentratorRadioTaskParams;
Task_Struct concentratorRadioTask; /* not static so you can see in ROV */
static uint8_t concentratorRadioTaskStack[CONCENTRATORRADIO_TASK_STACK_SIZE];
Event_Struct radioOperationEvent;  /* not static so you can see in ROV */
static Event_Handle radioOperationEventHandle;

static ConcentratorRadio_PacketReceivedCallback packetReceivedCallback;
static union ConcentratorPacket latestRxPacket;
static EasyLink_TxPacket txPacket;
static struct AckPacket ackPacket;
static uint8_t concentratorAddress;
static int8_t latestRssi;


/***** Prototypes *****/
static void concentratorRadioTaskFunction(UArg arg0, UArg arg1);
static void rxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status);
static void notifyPacketReceived(union ConcentratorPacket* latestRxPacket);
static void sendAck(uint8_t latestSourceAddress);

/***** Function definitions *****/
void radioReceive_init(void) {

	/* Create event used internally for state changes */
	Event_Params eventParam;
	Event_Params_init(&eventParam);
	Event_construct(&radioOperationEvent, &eventParam);
	radioOperationEventHandle = Event_handle(&radioOperationEvent);

	/* Create the concentrator radio protocol task */
	Task_Params_init(&concentratorRadioTaskParams);
	concentratorRadioTaskParams.stackSize = CONCENTRATORRADIO_TASK_STACK_SIZE;
	concentratorRadioTaskParams.priority = CONCENTRATORRADIO_TASK_PRIORITY;
	concentratorRadioTaskParams.stack = &concentratorRadioTaskStack;
	Task_construct(&concentratorRadioTask, concentratorRadioTaskFunction, &concentratorRadioTaskParams, NULL);
}

// configure the callback to alpha or gateway
void ConcentratorRadioTask_registerPacketReceivedCallback(ConcentratorRadio_PacketReceivedCallback callback) {
	packetReceivedCallback = callback;
}

static void concentratorRadioTaskFunction(UArg arg0, UArg arg1)
{
	/* Initialize EasyLink */
	if(EasyLink_init(RADIO_EASYLINK_MODULATION) != EasyLink_Status_Success) {
		System_abort("EasyLink_init failed");
	}

	System_printf("Starting Radio Receive!\n");
	/* If you wish to use a frequency other than the default use
	 * the below API
	 * EasyLink_setFrequency(868000000);
	 */

	/* Set src address of ACK packet */

	/* int buildType; */
	/* int result = varFromConfigInt("tagType",&buildType); */

	// tmp
	/* buildType = TAG_TYPE; */

	//	System_printf("buildType = %i\n",buildType);
	//
	//	if(buildType == 1){
	//		concentratorAddress = ALPHA_ADDRESS;
	//	} else if ( buildType == 3 ){
	//		concentratorAddress = GATEWAY_ADDRESS;
	//	} else {
	//		System_printf("buildType ERROR");
	//		concentratorAddress = GATEWAY_ADDRESS;
	//	}*/


	/* Set src address of ACK packet */;
	concentratorAddress = GATEWAY_ADDRESS;

	EasyLink_enableRxAddrFilter(NULL, 0, 0); // address filtering is disabled for A and G

	/* Set up Ack packet */
	ackPacket.header.sourceAddress = concentratorAddress;
	ackPacket.header.packetType = RADIO_PACKET_TYPE_ACK_PACKET;

	/* Wait for sensor packet (and save to memory) */
	if(EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success) {
		System_abort("EasyLink_receiveAsync failed");
	}

	while (1) {
		uint32_t events = Event_pend(radioOperationEventHandle, 0, RADIO_EVENT_ALL, BIOS_WAIT_FOREVER);

		/* If valid packet received */
		if(events & RADIO_EVENT_VALID_PACKET_RECEIVED) {

			//System_printf("Valid Packet, sending ACK...\n");

			/* Send the ack packet */
			sendAck(latestRxPacket.header.sourceAddress);

			/* Call packet received callback */
			notifyPacketReceived(&latestRxPacket);

			//System_printf("ACK sent. Back to listening\n");

			/* Go back to RX (to wait to sensor packet) */
			if(EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success) {
				System_abort("EasyLink_receiveAsync failed: 1");
			}

			/* toggle Activity LED */
			PIN_setOutputValue(ledPinHandle, CONCENTRATOR_ACTIVITY_LED,
					!PIN_getOutputValue(CONCENTRATOR_ACTIVITY_LED));
		}

		/* If invalid packet received */
		if(events & RADIO_EVENT_INVALID_PACKET_RECEIVED) {

			System_printf("Invalid Packet. Back to listening.\n");
			/* Go back to RX */
			if(EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success) {
				System_abort("EasyLink_receiveAsync failed: 2");
			}
		}

	} // end while loop
}

/*send an ACK packet to the src addr of the RX'd sensor packet*/
static void sendAck(uint8_t latestSourceAddress) {

	/* Set destinationAdress */
	txPacket.dstAddr[0] = latestSourceAddress;

	/* Copy ACK packet to payload, skipping the destination adress byte.
	 * Note that the EasyLink API will implicitly both add the length byte and the destination address byte. */
	memcpy(txPacket.payload, &ackPacket.header, sizeof(ackPacket));
	txPacket.len = sizeof(ackPacket);

	/* Send packet  */
	if (EasyLink_transmit(&txPacket) != EasyLink_Status_Success)
	{
		System_abort("EasyLink_transmit failed: failed to send ACK");
	}
}

/*the callback is shared; defined elsewhere*/
static void notifyPacketReceived(union ConcentratorPacket* latestRxPacket)
{
	if (packetReceivedCallback)
	{
		packetReceivedCallback(latestRxPacket, latestRssi);
	}
}

/*callback for receive: wait for sensor packet, and check for src address of packet (for Alphas only)*/
static void rxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status)
{
	union ConcentratorPacket* tmpRxPacket;

	/* If we received a packet successfully */
	if (status == EasyLink_Status_Success)
	{
		/* Save the latest RSSI, which is later sent to the receive callback */
		latestRssi = (int8_t)rxPacket->rssi;

		/* Check that this is a valid packet */
		tmpRxPacket = (union ConcentratorPacket*)(rxPacket->payload);

		/* If this is a known packet */
		if (tmpRxPacket->header.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET)
		{
			// both alpha(passed check) and gateway do this
			/* Save packet */
			memcpy((void*)&latestRxPacket, &rxPacket->payload, sizeof(struct sensorPacket));
			/* Signal packet received */
			Event_post(radioOperationEventHandle, RADIO_EVENT_VALID_PACKET_RECEIVED);

		}else if(tmpRxPacket->header.packetType == RADIO_PACKET_TYPE_ACCEL_PACKET){


			// both alpha(passed check) and gateway do this
			/* Save packet */
			memcpy((void*)&latestRxPacket, &rxPacket->payload, sizeof(struct sensorPacket));
			/* Signal packet received */
			Event_post(radioOperationEventHandle, RADIO_EVENT_VALID_PACKET_RECEIVED);
		}
		else{
			// ignore unknown packet or ACK packet
			Event_post(radioOperationEventHandle, RADIO_EVENT_INVALID_PACKET_RECEIVED);
		}
	}	else // status = some error; packet not rx'd successfully
	{
		System_printf("Receive Error.\n");
		/* Signal invalid packet received */
		Event_post(radioOperationEventHandle, RADIO_EVENT_INVALID_PACKET_RECEIVED);
	}
}
