/*
 * eepromTest.c
 *
 *  Created on: Feb 10, 2017
 *      Author: Erik-PC
 */

/***** Includes *****/
#include <debug.h>
#include <config.h>
#include <radioProtocol.h>
#include <eepromTest.h>
#include <eeprom.h>
#include <serialize.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/***** Defines *****/
#define TASKSTACKSIZE		1024	//i2c

/***** Variable Declarations *****/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];
struct sampleData sampledata;

/***** Prototypes *****/
void eepromTestWriteReadNormal();
void eepromTestWriteReadAccelerometer();
void eepromTestValidateMemory();
void eepromTestGetNext();
void eepromTestGetNextWithWrap();
/*helpers*/
void eepromPrintSample(uint8_t *buf);

/***** Function Definition *****/
// test runner for the eeprom module
void eepromTest_init() {
	if(verbose_eepromTest){System_printf("Starting eepromTest\n");System_flush();}
	// start at beginning of memory addresses
	eeprom_reset();

	// start eeprom thread
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)eepromTestValidateMemory, &taskParams, NULL);

	if(verbose_eepromTest){System_printf("Finished eepromTest\n");System_flush();}
}

void eepromTestGetNext() {
	if(verbose_eepromTest){System_printf("[eepromTestGetNext]\n");System_flush();}

	unsigned testsize = 3;

	eeprom_reset();

	// write thrice
	unsigned iter = 0;
	while (iter < testsize) {
		eeprom_write(&sampledata);
		++iter;
	}

	// samples retieved from mem
	unsigned samplesGotten = 0;

	// get from sample set, and loop until no samples left
	struct sampleData sample2;
	eeprom_getNext(&sample2);
	++samplesGotten;

	while (!eeprom_getNext(&sample2)) { ++samplesGotten; }

	// validate
	if (samplesGotten == testsize) {
		if(verbose_eepromTest){System_printf("%d samples gotten! Correct!\n", testsize);System_flush();}
	} else {
		if(verbose_eepromTest){System_printf("ERR: failure to retrieve samples using getNext(): %d\n", samplesGotten);System_flush();}
	}
}

void eepromTestGetNextWithWrap() {
	if(verbose_eepromTest){System_printf("[eepromTestGetNextWithWrap]\n");System_flush();}

	unsigned testsize = 5;

	// test packet
	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.tempData.temp_h = 0x78;
	sampledata.tempData.temp_l = 0x65;
	sampledata.heartRateData.rate_h = 0x90;
	sampledata.heartRateData.rate_l = 0x87;
	sampledata.heartRateData.temp_h = 0x45;
	sampledata.heartRateData.temp_l = 0x32;
	sampledata.error = 0x0;

	eeprom_reset();

	// write thrice
	unsigned iter = 0;
	while (iter < testsize) {
		eeprom_write(&sampledata);
		++iter;
	}

	// samples retieved from mem
	unsigned samplesGotten = 0;

	// get from sample set, and loop until no samples left
	struct sampleData sample2;
	eeprom_getNext(&sample2);
	++samplesGotten;

	while (!eeprom_getNext(&sample2)) { ++samplesGotten; }

	// validate
	if (samplesGotten == MAX_EEPROM_ADDRESS / SAMPLE_SIZE) {
		if(verbose_eepromTest){System_printf("%d eeprom samples written/read!\n", MAX_EEPROM_ADDRESS / SAMPLE_SIZE);System_flush();}
	} else {
		if(verbose_eepromTest){System_printf("ERR: failure to retrieve samples using getNextWithWrap(): %d\n", samplesGotten);System_flush();}
	}
}

// Pre-condition:  eeprom address set to 0x0000
// Expects:        SAMPLE_SIZE bytes written and read from eeprom
// Post-condition: SAMPLE_SIZE bytes stored in eeprom
void eepromTestWriteReadNormal() {
	if(verbose_eepromTest){System_printf("[eepromTestWriteReadNormal]\n");System_flush();}

	// test packet
	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.tempData.temp_h = 0x78;
	sampledata.tempData.temp_l = 0x65;
	sampledata.heartRateData.rate_h = 0x90;
	sampledata.heartRateData.rate_l = 0x87;
	sampledata.heartRateData.temp_h = 0x45;
	sampledata.heartRateData.temp_l = 0x32;
	sampledata.error = 0x0;

	eeprom_reset();
	eeprom_write(&sampledata);

	// recreate packet from serialized bytes
	struct sampleData sample2;
	eeprom_getNext(&sample2);

	// verify unserialized packet
	bool success = (sampledata.cowID == sample2.cowID);
	success = (sampledata.packetType == sample2.packetType);
	success = (sampledata.timestamp == sample2.timestamp);
	success = (sampledata.tempData.temp_h == sample2.tempData.temp_h);
	success = (sampledata.tempData.temp_l == sample2.tempData.temp_l);
	success = (sampledata.heartRateData.rate_h == sample2.heartRateData.rate_h);
	success = (sampledata.heartRateData.rate_l == sample2.heartRateData.rate_l);
	success = (sampledata.heartRateData.temp_h == sample2.heartRateData.temp_h);
	success = (sampledata.heartRateData.temp_l == sample2.heartRateData.temp_l);
	success = (sampledata.error == sample2.error);

	if (success) {
		if(verbose_eepromTest){System_printf("eeprom sample successfully written/read\n");System_flush();}
	} else {
		if(verbose_eepromTest){System_printf("ERR: eeprom sample failed to write/read!\n");System_flush();}
	}
}

// Pre-condition:  eeprom address set to 0x0000
// Expects:        SAMPLE_SIZE bytes written and read from eeprom
// Post-condition: SAMPLE_SIZE bytes stored in eeprom
void eepromTestWriteReadAccelerometer() {
	if(verbose_eepromTest){System_printf("[eepromTestWriteReadAccelerometer]\n");System_flush();}

	// test packet
	struct sampleData sample;
	sample.cowID = 1;
	sample.packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
	sample.timestamp = 0x12345678;
	sample.accelerometerData.x = 0x78;
	sample.accelerometerData.y = 0x89;
	sample.accelerometerData.z = 0x90;
	sample.error = 0x0;

	eeprom_reset();
	eeprom_write(&sample);

	// recreate packet from serialized bytes
	struct sampleData sample2;
	eeprom_getNext(&sample2);

	System_printf("ID: %d\n", sample2.cowID);
	System_printf("X: %x\n", sample2.accelerometerData.x);
	System_printf("Y: %x\n", sample2.accelerometerData.y);
	System_printf("Z: %x\n", sample2.accelerometerData.z);

	// verify unserialized packet
	bool success = (sampledata.cowID == sample2.cowID);
	success = (sample.packetType == sample2.packetType);
	success = (sample.timestamp == sample2.timestamp);
	success = (sample.accelerometerData.x == sample2.accelerometerData.x);
	success = (sample.accelerometerData.y == sample2.accelerometerData.y);
	success = (sample.accelerometerData.z == sample2.accelerometerData.z);
	success = (sample.error == sample2.error);

	if (success) {
		if(verbose_eepromTest){System_printf("eeprom sample successfully written/read\n");System_flush();}
	} else {
		if(verbose_eepromTest){System_printf("ERR: eeprom sample failed to write/read!\n");System_flush();}
	}
}

// Pre-condition:  eeprom address set to 0x0000
// Expects:        all eeprom addresses written and read
// Post-condition: eeprom full of test data
void eepromTestValidateMemory() {
	if(verbose_eepromTest){System_printf("[eepromTestValidateMemory]\n");System_flush();}
	System_flush();

	// test settings
	uint16_t testsize = MAX_EEPROM_ADDRESS / SAMPLE_SIZE;

	// tally bad bytes
	unsigned badWrites = 0;
	unsigned badReads = 0;

	// test packet
	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.tempData.temp_h = 0x78;
	sampledata.tempData.temp_l = 0x65;
	sampledata.heartRateData.rate_h = 0x90;
	sampledata.heartRateData.rate_l = 0x87;
	sampledata.heartRateData.temp_h = 0x45;
	sampledata.heartRateData.temp_l = 0x32;
	sampledata.error = 0x0;

	// reset memory pointer
	eeprom_reset();

	// check writes
	for (; eeprom_currentAddress < testsize;) {
		if (!eeprom_write(&sampledata)) {
			badWrites++;
		}
	}

	// read packet
	struct sampleData sample2;
	sample2.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;

	// check reads
	for (; eeprom_currentAddress < testsize;) {
		eeprom_getNext(&sample2);

		// verify unserialized packet
		bool success = (sampledata.cowID == sample2.cowID);
		success = (sampledata.packetType == sample2.packetType);
		success = (sampledata.timestamp == sample2.timestamp);
		success = (sampledata.tempData.temp_h == sample2.tempData.temp_h);
		success = (sampledata.tempData.temp_l == sample2.tempData.temp_l);
		success = (sampledata.heartRateData.rate_h == sample2.heartRateData.rate_h);
		success = (sampledata.heartRateData.rate_l == sample2.heartRateData.rate_l);
		success = (sampledata.heartRateData.temp_h == sample2.heartRateData.temp_h);
		success = (sampledata.heartRateData.temp_l == sample2.heartRateData.temp_l);
		success = (sampledata.error == sample2.error);

		if (!success) {
			++badReads;
		}
	}

	// count successful writes/reads
	if(verbose_eepromTest){System_printf("Good Writes = %d / %d\n", testsize - badWrites, testsize);System_flush();}
	if(verbose_eepromTest){System_printf("Good Reads = %d / %d\n", testsize - badReads, testsize);System_flush();}
}

void eepromPrintSample(uint8_t *buf) {
	int i;
	for (i = 0; i < SAMPLE_SIZE; ++i) {
		System_printf("0x%02x ", buf[i]);
	}
	System_printf("\n");
}
