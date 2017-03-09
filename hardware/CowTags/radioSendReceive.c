/*
 * radioSendReceive.c
 *
 *  Created on: Feb 16, 2017
 *      Author: Erik-PC
 */

/***** Includes *****/
#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Clock.h>
#include <xdc/runtime/Timestamp.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

/* Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>
#include <radioSendReceive.h>
#include "easylink/EasyLink.h"
#include "radioProtocol.h"

/* Board Header files */
#include <Board.h>
#include <debug.h>
#include "pinTable.h"
#include <config_parse.h>
#include <stdlib.h>
#include <driverlib/trng.h>
#include <driverlib/aon_batmon.h>

/***** Defines *****/
#define ALPHARADIO_TASK_STACK_SIZE 1024
#define ALPHARADIO_TASK_PRIORITY   3

#define ALPHARADIO_EVENT_ALL                 0xFFFFFFFF
#define ALPHARADIO_EVENT_SEND_DATA      	(uint32_t)(1 << 0)
#define ALPHARADIO_EVENT_DATA_ACK_RECEIVED   (uint32_t)(1 << 1)
#define ALPHARADIO_EVENT_ACK_TIMEOUT         (uint32_t)(1 << 2)
#define ALPHARADIO_EVENT_SEND_FAIL           (uint32_t)(1 << 3)
#define ALPHARADIO_EVENT_VALID_PACKET_RECEIVED      (uint32_t)(1 << 4)
#define ALPHARADIO_EVENT_INVALID_PACKET_RECEIVED (uint32_t)(1 << 5)

#define ALPHARADIO_MAX_RETRIES 6
#define ALPHARADIO_ACK_TIMEOUT_TIME_MS (500)

/***** Type declarations *****/
struct RadioOperation {
	EasyLink_TxPacket easyLinkTxPacket;
	uint8_t retriesDone;
	uint8_t maxNumberOfRetries;
	uint32_t ackTimeoutMs;
	enum alphaRadioOperationStatus result;
};

/***** Variable declarations *****/
static Task_Params alphaRadioTaskParams;
Task_Struct alphaRadioTask;        /* not static so you can see in ROV */
static uint8_t alphaRadioTaskStack[ALPHARADIO_TASK_STACK_SIZE];
Semaphore_Struct radioAccessSem;  /* not static so you can see in ROV */
static Semaphore_Handle radioAccessSemHandle;
Event_Struct radioOperationEvent; /* not static so you can see in ROV */
static Event_Handle radioOperationEventHandle;
Semaphore_Struct radioResultSem;  /* not static so you can see in ROV */
static Semaphore_Handle radioResultSemHandle;

static struct RadioOperation currentRadioOperation;
static uint8_t nodeAddress = 0; // ending in 0
static uint8_t concentratorAddress;

static struct sensorPacket sensorPacket;
static struct sampleData sampledata;

static ConcentratorRadio_PacketReceivedCallback packetReceivedCallback;
static union ConcentratorPacket latestRxPacket;
static EasyLink_TxPacket txPacket;
static struct AckPacket ackPacket;

static int8_t latestRssi;

/***** Prototypes *****/
static void alphaRadioTaskFunction(UArg arg0, UArg arg1);

static void sendAlphaPacket(struct sensorPacket betaPacket, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs);
static void resendPacket();
static void notifyPacketReceived(union ConcentratorPacket* latestRxPacket);
static void sendAck(uint8_t latestSourceAddress);
static void returnRadioOperationStatus(enum alphaRadioOperationStatus status);

static void rxDoneCallbackSend(EasyLink_RxPacket * rxPacket, EasyLink_Status status);
static void rxDoneCallbackReceive(EasyLink_RxPacket * rxPacket, EasyLink_Status status);

/***** Function definitions *****/
void radioSendReceive_init(void) {
	/* Create semaphore used for exclusive radio access */
	Semaphore_Params semParam;
	Semaphore_Params_init(&semParam);
	Semaphore_construct(&radioAccessSem, 1, &semParam);
	radioAccessSemHandle = Semaphore_handle(&radioAccessSem);

	/* Create semaphore used for callers to wait for result */
	Semaphore_construct(&radioResultSem, 0, &semParam);
	radioResultSemHandle = Semaphore_handle(&radioResultSem);

	/* Create event used internally for state changes */
	Event_Params eventParam;
	Event_Params_init(&eventParam);
	Event_construct(&radioOperationEvent, &eventParam);
	radioOperationEventHandle = Event_handle(&radioOperationEvent);

	/* Create the radio protocol task */
	Task_Params_init(&alphaRadioTaskParams);
	alphaRadioTaskParams.stackSize = ALPHARADIO_TASK_STACK_SIZE;
	alphaRadioTaskParams.priority = ALPHARADIO_TASK_PRIORITY;
	alphaRadioTaskParams.stack = &alphaRadioTaskStack;
	Task_construct(&alphaRadioTask, alphaRadioTaskFunction, &alphaRadioTaskParams, NULL);
}

static void alphaRadioTaskFunction(UArg arg0, UArg arg1)
{
	/* Initialize EasyLink */
	if(EasyLink_init(RADIO_EASYLINK_MODULATION) != EasyLink_Status_Success) {
		System_abort("EasyLink_init failed");
	}

	System_printf("Starting alpha Radio thread\n");

	nodeAddress = ALPHA_ADDRESS;
	concentratorAddress = ALPHA_ADDRESS;

	/* Setup header */
	sensorPacket.header.sourceAddress = nodeAddress;

	/* Set up Ack packet */
	ackPacket.header.sourceAddress = concentratorAddress;
	ackPacket.header.packetType = RADIO_PACKET_TYPE_ACK_PACKET;

	EasyLink_enableRxAddrFilter(NULL, 0, 0); // address filtering is disabled for A and G

	/* Wait for sensor packet (and save to memory) */
	if(EasyLink_receiveAsync(rxDoneCallbackReceive, 0) != EasyLink_Status_Success) {
		System_abort("EasyLink_receiveAsync failed");
	}

	/* Enter main task loop */
	while (1)
	{
		//uint32_t events = Event_pend(radioOperationEventHandle, 0, ALPHARADIO_EVENT_ALL, BIOS_WAIT_FOREVER);
		uint32_t events = Event_pend(radioOperationEventHandle, 0, ALPHARADIO_EVENT_ALL, 0);

		/* If valid packet received */
		if(events & ALPHARADIO_EVENT_VALID_PACKET_RECEIVED) {

			System_printf("RadioReceive: Valid Packet, sending ACK...\n");

			/* Send the ack packet */
			sendAck(latestRxPacket.header.sourceAddress);

			/* Call packet received callback */
			notifyPacketReceived(&latestRxPacket);

			System_printf("RadioReceive: ACK sent. Back to listening\n");

			/* Go back to RX (to wait to sensor packet) */
			if(EasyLink_receiveAsync(rxDoneCallbackReceive, 0) != EasyLink_Status_Success) {
				//System_abort("EasyLink_receiveAsync failed: ALPHARADIO_EVENT_VALID_PACKET_RECEIVED");
				System_printf("EasyLink_receiveAsync failed: ALPHARADIO_EVENT_VALID_PACKET_RECEIVED\n");
			}

			/* toggle Activity LED */
			PIN_setOutputValue(ledPinHandle, CONCENTRATOR_ACTIVITY_LED,
					!PIN_getOutputValue(CONCENTRATOR_ACTIVITY_LED));
		}

		/* If invalid packet received */
		if(events & ALPHARADIO_EVENT_INVALID_PACKET_RECEIVED) {

			System_printf("RadioReceive: invalid Packet, back to listening.\n");
			/* Go back to RX */
			if(EasyLink_receiveAsync(rxDoneCallbackReceive, 0) != EasyLink_Status_Success) {
				//System_abort("EasyLink_receiveAsync failed: ALPHARADIO_EVENT_INVALID_PACKET_RECEIVED");
				System_printf("EasyLink_receiveAsync failed: ALPHARADIO_EVENT_INVALID_PACKET_RECEIVED\n");
			}
		}

		/* If we should send data */
		if (events & ALPHARADIO_EVENT_SEND_DATA)
		{
			sensorPacket.sampledata = sampledata;
			sensorPacket.header.packetType = sampledata.packetType;
			System_printf("RadioSend: sending data...\n");
			sendAlphaPacket(sensorPacket, ALPHARADIO_MAX_RETRIES, ALPHARADIO_ACK_TIMEOUT_TIME_MS);

		}

		/* If we get an ACK from the concentrator */
		if (events & ALPHARADIO_EVENT_DATA_ACK_RECEIVED)
		{
			returnRadioOperationStatus(AlphaRadioStatus_Success);
			System_printf("RadioSend: ACK RECEIVED! Transmission successful.\n");
		}

		/* If we get an ACK timeout */
		if (events & ALPHARADIO_EVENT_ACK_TIMEOUT)
		{
			/* If we haven't resent it the maximum number of times yet, then resend packet */
			if (currentRadioOperation.retriesDone < currentRadioOperation.maxNumberOfRetries)
			{
				resendPacket();
				System_printf("RadioSend: Timed out! resending for the %d th time\n", currentRadioOperation.retriesDone);
			}
			else
			{
				/* Else return send fail */
				returnRadioOperationStatus(AlphaRadioStatus_Failed);
			}
		}

		/* If send fail */
		if (events & ALPHARADIO_EVENT_SEND_FAIL)
		{
			System_printf("RadioSend: Send failed\n");
			returnRadioOperationStatus(AlphaRadioStatus_FailedNotConnected);
		}
	}
}


enum alphaRadioOperationStatus alphaRadioSendData(struct sampleData data){
	enum alphaRadioOperationStatus status;

	/* Get radio access semaphore */
	Semaphore_pend(radioAccessSemHandle, BIOS_WAIT_FOREVER);

	/* Save data to send */
	sampledata = data;

	/* Raise RADIO_EVENT_SEND_DATA event */
	Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_SEND_DATA);

	/* Wait for result */
	Semaphore_pend(radioResultSemHandle, BIOS_WAIT_FOREVER);

	/* Get result */
	status = currentRadioOperation.result;

	/* Return radio access semaphore */
	Semaphore_post(radioAccessSemHandle);

	return status;
}

static void returnRadioOperationStatus(enum alphaRadioOperationStatus result)
{
	/* Save result */
	currentRadioOperation.result = result;

	/* Post result semaphore */
	Semaphore_post(radioResultSemHandle);
}

static void sendAlphaPacket(struct sensorPacket bp, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs){

	//sends to alpha and gateway
	/*
	if( Timestamp_get32() % 2 ){
		currentRadioOperation.easyLinkTxPacket.dstAddr[0] = 0x01;
	}else{
		currentRadioOperation.easyLinkTxPacket.dstAddr[0] = 0x02;
	}*/
	//System_printf("sending to alpha's or gateway\n");
	currentRadioOperation.easyLinkTxPacket.dstAddr[0] = GATEWAY_ADDRESS;

	/* Copy packet to payload */
	memcpy(currentRadioOperation.easyLinkTxPacket.payload, ((uint8_t*)&sensorPacket), sizeof(struct sensorPacket));

	currentRadioOperation.easyLinkTxPacket.len = sizeof(struct sensorPacket);

	/* Setup retries */
	currentRadioOperation.maxNumberOfRetries = maxNumberOfRetries;
	currentRadioOperation.ackTimeoutMs = ackTimeoutMs;
	currentRadioOperation.retriesDone = 0;
	EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, EasyLink_ms_To_RadioTime(ackTimeoutMs));

	/* Send packet  */
	if (EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket) != EasyLink_Status_Success)
	{
		//System_abort("EasyLink_transmit failed: failed to send packet");
		System_printf("EasyLink_transmit failed: failed to send packet\n");
	}

	/* Enter RX */
	if (EasyLink_receiveAsync(rxDoneCallbackSend, 0) != EasyLink_Status_Success)
	{
		//System_abort("EasyLink_receiveAsync failed");
		System_printf("EasyLink_receiveAsync failed: send\n");
	}
}

static void resendPacket()
{
	/* Send packet  */
	if (EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket) != EasyLink_Status_Success)
	{
		//System_abort("EasyLink_transmit failed: failed to RESEND packet");
		System_printf("EasyLink_transmit failed: failed to RESEND packet\n");

	}

	/* Enter RX and wait for ACK with timeout */
	if (EasyLink_receiveAsync(rxDoneCallbackSend, 0) != EasyLink_Status_Success)
	{
		//System_abort("EasyLink_receiveAsync failed");
		System_printf("EasyLink_receiveAsync failed: resend\n");
	}

	/* Increase retries by one */
	currentRadioOperation.retriesDone++;
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
		//System_abort("EasyLink_transmit failed: failed to send ACK");
		System_printf("EasyLink_transmit failed: failed to send ACK\n");
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

// configure the callback to alpha or gateway
void AlphaRadioTask_registerPacketReceivedCallback(ConcentratorRadio_PacketReceivedCallback callback) {
	packetReceivedCallback = callback;
}


/*callback for send: wait for ACK packet*/
static void rxDoneCallbackSend(EasyLink_RxPacket * rxPacket, EasyLink_Status status)
{
	struct PacketHeader* packetHeader;

	/* If this callback is called because of a packet received */
	if (status == EasyLink_Status_Success)
	{
		/* Check the payload header */
		packetHeader = (struct PacketHeader*)rxPacket->payload;

		/* Check if this is an ACK packet */
		if (packetHeader->packetType == RADIO_PACKET_TYPE_ACK_PACKET)
		{
			/* Signal ACK packet received */
			Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_DATA_ACK_RECEIVED);
		}
		else
		{
			/* Packet Error, treat as a Timeout and Post a RADIO_EVENT_ACK_TIMEOUT
               event */
			Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_ACK_TIMEOUT);
		}
	}
	/* did the Rx timeout */
	else if(status == EasyLink_Status_Rx_Timeout)
	{
		/* Post a RADIO_EVENT_ACK_TIMEOUT event */
		Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_ACK_TIMEOUT);
	}
	else
	{
		/* The Ack reception may have been corrupted causing an error.
		 * Treat this as a timeout
		 */
		Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_SEND_FAIL);
	}
}

/*callback for receive: wait for sensor packet, and check for src address of packet (for Alphas only)*/
static void rxDoneCallbackReceive(EasyLink_RxPacket * rxPacket, EasyLink_Status status)
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

			// alpha check
			if( (concentratorAddress & 0x3) == ALPHA_ADDRESS){ // IF I AM AN ALPHA
				if((tmpRxPacket->header.sourceAddress & 0x1) == 1){ // IF THE SOURCE IS ANOTHER CONCENTRATOR
					// ignore the alpha packet
					Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_INVALID_PACKET_RECEIVED);
					return;
				}
			}

			// both alpha(passed check) and gateway do this
			/* Save packet */
			memcpy((void*)&latestRxPacket, &rxPacket->payload, sizeof(struct sensorPacket));
			/* Signal packet received */
			Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_VALID_PACKET_RECEIVED);

		}else if(tmpRxPacket->header.packetType == RADIO_PACKET_TYPE_ACCEL_PACKET){

						// alpha check
						if( (concentratorAddress & 0x3) == ALPHA_ADDRESS){ // IF I AM AN ALPHA
							if((tmpRxPacket->header.sourceAddress & 0x1) == 1){ // IF THE SOURCE IS ANOTHER CONCENTRATOR
								// ignore the alpha packet
								Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_INVALID_PACKET_RECEIVED);
								return;
							}
						}

						// both alpha(passed check) and gateway do this
						/* Save packet */
						memcpy((void*)&latestRxPacket, &rxPacket->payload, sizeof(struct sensorPacket));
						/* Signal packet received */
						Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_VALID_PACKET_RECEIVED);
		}
		else if(tmpRxPacket->header.packetType == RADIO_PACKET_TYPE_ACK_PACKET){
			//alpha sends now, so it needs ACK packets
			Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_DATA_ACK_RECEIVED);
		}
		else{
			// ignore unknown packet or ACK packet
			Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_INVALID_PACKET_RECEIVED);
		}
	}	else // status = some error; packet not rx'd successfully
	{
		//System_printf("Receive Error.\n");
		/* Signal invalid packet received */
		Event_post(radioOperationEventHandle, ALPHARADIO_EVENT_SEND_FAIL);
	}
}
