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


/***** Defines *****/
#define NODE_TASK_STACK_SIZE 1024
#define NODE_TASK_PRIORITY   3

#define NODE_EVENT_ALL                  0xFFFFFFFF
#define NODE_EVENT_NEW_VALUE    (uint32_t)(1 << 0)

/* A change mask of 0xFF0 means that changes in the lower 4 bits does not trigger a wakeup. */
//#define NODE_ADCTASK_CHANGE_MASK                    0xFF0

/* Minimum slow Report interval is 50s (in units of samplingTime)*/
//#define NODE_ADCTASK_REPORTINTERVAL_SLOW                50
/* Minimum fast Report interval is 1s (in units of samplingTime) for 30s*/
//#define NODE_ADCTASK_REPORTINTERVAL_FAST                1
//#define NODE_ADCTASK_REPORTINTERVAL_FAST_DURIATION_MS   30000



/***** Variable declarations *****/
static Task_Params nodeTaskParams;
Task_Struct nodeTask;    /* not static so you can see in ROV */
static uint8_t nodeTaskStack[NODE_TASK_STACK_SIZE];
Event_Struct nodeEvent;  /* not static so you can see in ROV */
static Event_Handle nodeEventHandle;

/* Clock for the fast report timeout */
//Clock_Struct fastReportTimeoutClock;     /* not static so you can see in ROV */
//static Clock_Handle fastReportTimeoutClockHandle;

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

	/* Create clock object which is used for fast report timeout */
	/*Clock_Params clkParams;
	clkParams.period = 0;
	clkParams.startFlag = FALSE;
	Clock_construct(&fastReportTimeoutClock, fastReportTimeoutCallback, 1, &clkParams);
	fastReportTimeoutClockHandle = Clock_handle(&fastReportTimeoutClock);*/

	/* Create the node task */
	Task_Params_init(&nodeTaskParams);
	nodeTaskParams.stackSize = NODE_TASK_STACK_SIZE;
	nodeTaskParams.priority = NODE_TASK_PRIORITY;
	nodeTaskParams.stack = &nodeTaskStack;
	Task_construct(&nodeTask, nodeTaskFunction, &nodeTaskParams, NULL);
}


static void nodeTaskFunction(UArg arg0, UArg arg1)
{
	/* setup timeout for fast report timeout */
	//Clock_setTimeout(fastReportTimeoutClockHandle,
	//		NODE_ADCTASK_REPORTINTERVAL_FAST_DURIATION_MS * 1000 / Clock_tickPeriod);

	/* start fast report and timeout */
	//Clock_start(fastReportTimeoutClockHandle);

	while(1) {
		sampledata.tempData = getObjTemp();
		sampledata.accelerometerData = getAcceleration();
		sampledata.heartRateData = getHeartRate();

		int delay = 10000;
		CPUdelay(delay*1000);

		/* Toggle activity LED */
		PIN_setOutputValue(ledPinHandle, NODE_ACTIVITY_LED, !PIN_getOutputValue(NODE_ACTIVITY_LED) );

		/* Send value to concentrator */
		betaRadioSendData(sampledata);
		if(verbose_antennas){
						System_printf("BetaRadio: sent packet with Temp_Data = %i.%i, "
																  "Acc_Data= x=%i y=%i z=%i, "
								      	  	  	  	  	  	  	  "IR_Data_H = %i, IR_Data_L = %i \n",
																  sampledata.tempData.temp_h,
																  sampledata.tempData.temp_l,
																  sampledata.accelerometerData.x,
																  sampledata.accelerometerData.y,
																  sampledata.accelerometerData.z,
																  sampledata.heartRateData.rate_h,
																  sampledata.heartRateData.rate_l);
						System_flush();
		}
	}
}

//void betaCallBack(struct sampleData newsampledata){
//	sampledata = newsampledata;
//	Event_post(nodeEventHandle, NODE_EVENT_NEW_VALUE);
//}

/*void fastReportTimeoutCallback(UArg arg0)
{
	//stop fast report
	SceAdc_setReportInterval(NODE_ADCTASK_REPORTINTERVAL_SLOW, NODE_ADCTASK_CHANGE_MASK);
}*/
