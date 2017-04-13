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
#include <radioProtocol.h>
#include <betaRadioTest.h>
#include <stdio.h>

/* XDCtools Header files */
#include <xdc/runtime/System.h>

#include <xdc/runtime/Timestamp.h>

/* BIOS Header files */
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>

/* Drivers */
#include <ti/drivers/PIN.h>
#include <RadioSend.h>
#include <sensors.h>
#include <eeprom.h>
#include <Sleep.h>

/* Board Header files */
#include <Board.h>
#include <pinTable.h>

/***** Defines *****/
#define BETARADIOTEST_TASK_STACK_SIZE 8192
#define BETARADIOTEST_TASK_PRIORITY   3

/***** Variable Declarations *****/
static Task_Params betaRadioTestTaskParams;
Task_Struct betaRadioTestTask;    /* not static so you can see in ROV */
static uint8_t betaRadioTestTaskStack[BETARADIOTEST_TASK_STACK_SIZE];

/***** Prototypes *****/
static void betaRadioTestTaskFunction(UArg arg0, UArg arg1);
void printSampleData(struct sampleData sampleData);

/***** Function Definitions *****/
void betaRadioTest_init(void)
{
	/* Create the betaRadioTest task */
	Task_Params_init(&betaRadioTestTaskParams);
	betaRadioTestTaskParams.stackSize = BETARADIOTEST_TASK_STACK_SIZE;
	betaRadioTestTaskParams.priority = BETARADIOTEST_TASK_PRIORITY;
	betaRadioTestTaskParams.stack = &betaRadioTestTaskStack;
	Task_construct(&betaRadioTestTask, betaRadioTestTaskFunction, &betaRadioTestTaskParams, NULL);
}

static void betaRadioTestTaskFunction(UArg arg0, UArg arg1)
{
	if(verbose_betaRadioTest){System_printf("Initializing betaRadioTest...\n");System_flush();}

	while(1){
		struct sampleData sampledata;
		enum NodeRadioOperationStatus results;

		sampledata.cowID = 5;
		sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
		sampledata.timestamp = 0x12345678;
		sampledata.error = 0x0;

		if(ignoreSensors){
			if(verbose_betaRadioTest){System_printf("Ignoring sensors, making fake packets\n");System_flush();}
			sampledata.tempData.temp_h = 0x78;
			sampledata.tempData.temp_l = 0x65;
			sampledata.heartRateData.rate_h = 0x90;
			sampledata.heartRateData.rate_l = 0x87;
			sampledata.heartRateData.temp_h = 0x45;
			sampledata.heartRateData.temp_l = 0x32;
		} else {
			if(verbose_betaRadioTest){System_printf("Creating Packet...\n");System_flush();}
			makeSensorPacket(&sampledata);
			if(verbose_betaRadioTest){System_printf("Packet Created\n");System_flush();}
		}

		if(verbose_betaRadioTest) {
			printSampleData(sampledata);
			if (verbose_beta_log) {
				file_printSampleData(sampledata);
			}
		}

		// send packet or save to eeprom
		if(verbose_betaRadioTest){System_printf("BetaTest: sending packet...\n");System_flush();}

		results = betaRadioSendData(sampledata);
		if (results != NodeRadioStatus_Success) {

			if (usingEeprom) {
				if(verbose_betaRadioTest){System_printf("packet sent error, saving to eeprom: %i\n",results);System_flush();}
				eeprom_write(&sampledata);
			} else {
				if(verbose_betaRadioTest){System_printf("packet sent error, but not saving to eeprom\n");}
			}
		} else{

			// print sent data
			if(verbose_betaRadioTest) {
				printSampleData(sampledata);
			}
		}


		// attempt to send saved samples as well
		if (usingEeprom) {
			if(verbose_betaRadioTest){System_printf("check for saved samples to send\n");}
			bool isSending = true;
			int oldSamplesSent = 0;

			do {
				struct sampleData oldSample;

				// check for another stored sample
				bool hasNext = eeprom_getNext(&oldSample);
				System_printf("GET NEXT: %d\n", hasNext);
				if (hasNext) {
					Task_sleep(sleepASecond());
					System_printf("RESSEDING\n");
					printSampleData(oldSample);
					results = betaRadioSendData(oldSample);

					if (results != NodeRadioStatus_Success) {
						// write back to eeprom is send is a no go
						if(verbose_betaRadioTest){System_printf("error sending from eeprom, saving back: %i\n",results);System_flush();}
						eeprom_write(&oldSample);
						isSending = false;
					} else {
						++oldSamplesSent;
					}
				} else {
					isSending = false;
				}
			} while (isSending == true);

			if(verbose_betaRadioTest){System_printf("%d saved samples sent\n", oldSamplesSent);}
		}


		if (verbose_sleep) {
			System_printf("zZzZzZzZzZzZzZzZzZ\n");
			System_printf("Z going to sleep z\n");
			System_printf("zZzZzZzZzZzZzZzZzZ\n");
		}
		Task_sleep(sleepASecond());
	}
}

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
			sampledata.accelerometerData.x_h << 8 | sampledata.accelerometerData.x_l,
			sampledata.accelerometerData.y_h << 8 | sampledata.accelerometerData.y_l,
			sampledata.accelerometerData.z_h << 8 | sampledata.accelerometerData.z_l);
	}
}

void file_printSampleData(struct sampleData sampledata) {
	static bool file_is_initialized = false;
	static FILE *fp;

	if (!file_is_initialized) {
		fp = fopen("../beta_packet_output.txt", "w");
		file_is_initialized = true;
	}

//	fprintf(fp, "BetaRadio: sent packet with CowID = %i, PacketType: %i, "
//			"Timestamp: %i, Error: %i, ",
//			sampledata.cowID,
//			sampledata.packetType,
//			sampledata.timestamp,
//			sampledata.error);
	if(sampledata.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET){
		fprintf(fp, "InfraredData = %u\n",
				sampledata.heartRateData.rate_h << 8 | sampledata.heartRateData.rate_l);
	}
	else if(sampledata.packetType == RADIO_PACKET_TYPE_ACCEL_PACKET){
		fprintf(fp, "accelerometerData= x=%i, y=%i, z=%i\n",
				sampledata.accelerometerData.x_h << 8 + sampledata.accelerometerData.x_l,
				sampledata.accelerometerData.y_h << 8 + sampledata.accelerometerData.y_l,
				sampledata.accelerometerData.z_h << 8 + sampledata.accelerometerData.z_l);
	}
	else if(sampledata.packetType == RADIO_PACKET_TYPE_TEMP_PACKET){
		fprintf(fp, "ambient temperature= %i, object temperature= %i\n",
				sampledata.heartRateData.temp_h << 8 | sampledata.heartRateData.temp_l,
				sampledata.tempData.temp_h << 8 | sampledata.tempData.temp_l);
	}
}
