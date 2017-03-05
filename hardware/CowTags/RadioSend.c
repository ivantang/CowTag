
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

/* Board Header files */
#include "Board.h"

#include <stdlib.h>
#include <driverlib/trng.h>
#include <driverlib/aon_batmon.h>
#include <RadioSend.h>
#include <Sleep.h>
#include <EventManager.h>
#include "easylink/EasyLink.h"
#include "radioProtocol.h"

#include <debug.h>
#include <config_parse.h>

/***** Defines *****/
#define NODERADIO_TASK_STACK_SIZE 1024
#define NODERADIO_TASK_PRIORITY   3

#define NODERADIO_MAX_RETRIES 6
#define NORERADIO_ACK_TIMEOUT_TIME_MS (500)


/***** Type declarations *****/
struct RadioOperation {
	EasyLink_TxPacket easyLinkTxPacket;
	uint8_t retriesDone;
	uint8_t maxNumberOfRetries;
	uint32_t ackTimeoutMs;
	enum NodeRadioOperationStatus result;
};


/***** Variable declarations *****/
static Task_Params nodeRadioTaskParams;
Task_Struct nodeRadioTask;        /* not static so you can see in ROV */
static uint8_t nodeRadioTaskStack[NODERADIO_TASK_STACK_SIZE];
Semaphore_Struct radioAccessSem;  /* not static so you can see in ROV */
static Semaphore_Handle radioAccessSemHandle;
Semaphore_Struct radioResultSem;  /* not static so you can see in ROV */
static Semaphore_Handle radioResultSemHandle;
static Event_Handle * eventHandle;

static struct RadioOperation currentRadioOperation;
static uint8_t nodeAddress = 0; // ending in 0

static struct sensorPacket sensorPacket;
static struct sampleData sampledata;

/* Pin driver handle */
extern PIN_Handle ledPinHandle;

/***** Prototypes *****/
static void nodeRadioTaskFunction(UArg arg0, UArg arg1);
static void returnRadioOperationStatus(enum NodeRadioOperationStatus status);
static void resendPacket();
static void rxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status);

static void sendBetaPacket(struct sensorPacket betaPacket, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs);

/***** Function definitions *****/
void radioSend_init() {

	/* Create semaphore used for exclusive radio access */
	Semaphore_Params semParam;
	Semaphore_Params_init(&semParam);
	Semaphore_construct(&radioAccessSem, 1, &semParam);
	radioAccessSemHandle = Semaphore_handle(&radioAccessSem);

	/* Create semaphore used for callers to wait for result */
	Semaphore_construct(&radioResultSem, 0, &semParam);
	radioResultSemHandle = Semaphore_handle(&radioResultSem);

	/* Get master event handle for internal state changes */
	eventHandle = getEventHandle();

	/* Create the radio protocol task */
	Task_Params_init(&nodeRadioTaskParams);
	nodeRadioTaskParams.stackSize = NODERADIO_TASK_STACK_SIZE;
	nodeRadioTaskParams.priority = NODERADIO_TASK_PRIORITY;
	nodeRadioTaskParams.stack = &nodeRadioTaskStack;
	Task_construct(&nodeRadioTask, nodeRadioTaskFunction, &nodeRadioTaskParams, NULL);
}

//static uint16_t i = 0;
static void nodeRadioTaskFunction(UArg arg0, UArg arg1)
{
	/* Initialize EasyLink */
	if(EasyLink_init(RADIO_EASYLINK_MODULATION) != EasyLink_Status_Success) {
		System_abort("EasyLink_init failed");
	}

	System_printf("Starting Radio Send\n");

	/*
	int buildType;
	int result = varFromConfigInt("tagType",&buildType);
	System_printf("buildType = %i\n",buildType);

	if(buildType == 0){
		nodeAddress = BETA_ADDRESS;
	} else if ( buildType == 1 ){
		nodeAddress = ALPHA_ADDRESS;
	} else {
		System_printf("buildType ERROR");
		nodeAddress = BETA_ADDRESS;
	}*/

	nodeAddress = ALPHA_ADDRESS;
	//nodeAddress = BETA_ADDRESS;

	/* Set the filter to the generated random address */
	if (EasyLink_enableRxAddrFilter(&nodeAddress, 1, 1) != EasyLink_Status_Success)
	{
		System_abort("EasyLink_enableRxAddrFilter failed");
	}

	/* Setup header */
	sensorPacket.header.sourceAddress = nodeAddress;

	/* Enter main task loop */
	while (1)
	{
		/* Wait for an event */
		uint32_t events = Event_pend(*eventHandle, 0, RADIO_EVENT_ALL, BIOS_WAIT_FOREVER);

		/* If we should send data */
		if (events & RADIO_EVENT_SEND_DATA)
		{
			sensorPacket.sampledata = sampledata;
			sensorPacket.header.packetType = sampledata.packetType;
			//System_printf("Sending data...\n");
			sendBetaPacket(sensorPacket, NODERADIO_MAX_RETRIES, NORERADIO_ACK_TIMEOUT_TIME_MS);

		}

		/* If we get an ACK from the concentrator */
		if (events & RADIO_EVENT_DATA_ACK_RECEIVED)
		{
			returnRadioOperationStatus(NodeRadioStatus_Success);
			System_printf("ACK RECEIVED! Transmission successful.\n");
		}

		/* If we get an ACK timeout */
		if (events & RADIO_EVENT_ACK_TIMEOUT)
		{
			/* If we haven't resent it the maximum number of times yet, then resend packet */
			if (currentRadioOperation.retriesDone < currentRadioOperation.maxNumberOfRetries)
			{
				resendPacket();
				System_printf("Timed out: resending for the %d th time\n", currentRadioOperation.retriesDone);
			}
			else
			{
				/* Else return send fail */

				returnRadioOperationStatus(NodeRadioStatus_Failed);

			}
		}

		/* If send fail */
		if (events & RADIO_EVENT_SEND_FAIL)
		{
			returnRadioOperationStatus(NodeRadioStatus_FailedNotConnected);
		}

		/* if time to go to sleep */
		if (events & RADIO_EVENT_SLEEP)
		{
			System_printf("...Radio going to sleep...\n");
			Task_sleep(sleepAMinute());
			System_printf("...Radio waking up!\n");
		}
	}
}


enum NodeRadioOperationStatus betaRadioSendData(struct sampleData data){
	enum NodeRadioOperationStatus status;

	/* Get radio access semaphore */
	Semaphore_pend(radioAccessSemHandle, BIOS_WAIT_FOREVER);

	/* Save data to send */
	sampledata = data;

	/* Raise RADIO_EVENT_SEND_DATA event */
	Event_post(*eventHandle, RADIO_EVENT_SEND_DATA);

	/* Wait for result */
	Semaphore_pend(radioResultSemHandle, BIOS_WAIT_FOREVER);

	/* Get result */
	status = currentRadioOperation.result;

	/* Return radio access semaphore */
	Semaphore_post(radioAccessSemHandle);

	return status;
}

static void returnRadioOperationStatus(enum NodeRadioOperationStatus result)
{
	/* Save result */
	currentRadioOperation.result = result;

	/* Post result semaphore */
	Semaphore_post(radioResultSemHandle);
}



static void sendBetaPacket(struct sensorPacket bp, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs){

	//sends to alpha and gateway
	/*
	if( Timestamp_get32() % 2 ){
		currentRadioOperation.easyLinkTxPacket.dstAddr[0] = 0x01;
	}else{
		currentRadioOperation.easyLinkTxPacket.dstAddr[0] = 0x02;
	}*/
	System_printf("sending to alpha's or gateway\n");
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
		System_abort("EasyLink_transmit failed");
	}

	/* Enter RX */
	if (EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success)
	{
		System_abort("EasyLink_receiveAsync failed");
	}
}


static void resendPacket()
{
	/* Send packet  */
	if (EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket) != EasyLink_Status_Success)
	{
		System_abort("EasyLink_transmit failed");
	}

	/* Enter RX and wait for ACK with timeout */
	if (EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success)
	{
		System_abort("EasyLink_receiveAsync failed");
	}

	/* Increase retries by one */
	currentRadioOperation.retriesDone++;
}

/*callback for send: wait for ACK packet*/
static void rxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status)
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
			Event_post(*eventHandle, RADIO_EVENT_DATA_ACK_RECEIVED);
		}
		else
		{
			/* Packet Error, treat as a Timeout and Post a RADIO_EVENT_ACK_TIMEOUT
               event */
			Event_post(*eventHandle, RADIO_EVENT_ACK_TIMEOUT);
		}
	}
	/* did the Rx timeout */
	else if(status == EasyLink_Status_Rx_Timeout)
	{
		/* Post a RADIO_EVENT_ACK_TIMEOUT event */
		Event_post(*eventHandle, RADIO_EVENT_ACK_TIMEOUT);
	}
	else
	{
		/* The Ack reception may have been corrupted causing an error.
		 * Treat this as a timeout
		 */
		Event_post(*eventHandle, RADIO_EVENT_SEND_FAIL);
	}
}
