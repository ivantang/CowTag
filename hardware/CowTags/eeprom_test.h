/*
 * eeprom_test.h
 *
 *  Created on: Feb 3, 2017
 *      Author: champ
 */

#ifndef EEPROM_TEST_H_
#define EEPROM_TEST_H_

#include "eeprom.h"

/*constants*/
#define TASKSTACKSIZE		1024	//i2c

/*test prototypes*/
bool eeprom_testStart();
void eeprom_testWriteReadSample();
void eeprom_testValidateMemory();
void eeprom_testGetNext();
void eeprom_testGetNextWithWrap();

/*helpers*/
void eeprom_printSample(uint8_t *buf);

/**structures*/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

// test runner for the eeprom module
bool eeprom_testStart() {
	// start at beginning of memory addresses
	eeprom_reset();

	// start eeprom thread
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)eeprom_testValidateMemory, &taskParams, NULL);

	return true;
}

/*** tests ***/

void eeprom_testGetNext() {
	System_printf("[eeprom_testGetNext]\n");
	System_flush();

	unsigned testsize = 3;

	// test packet
	struct sensorPacket packet;
	packet.header.sourceAddress = 1;
	packet.header.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	packet.sampledata.tempData.timestamp = 0x12345678;
	packet.sampledata.tempData.temp_h = 0x5678;
	packet.sampledata.tempData.temp_l = 0x8765;
	packet.sampledata.heartRateData.rate_h = 0x7890;
	packet.sampledata.heartRateData.rate_l = 0x0987;
	packet.sampledata.heartRateData.temp_h = 0x2345;
	packet.sampledata.heartRateData.temp_l = 0x5432;

	eeprom_reset();

	// write thrice
	unsigned iter = 0;
	while (iter < testsize) {
		eeprom_write(&packet);
		++iter;
	}

	// samples retieved from mem
	unsigned samplesGotten = 0;

	// get from sample set, and loop until no samples left
	struct sensorPacket packet2;
	eeprom_getNext(&packet2);
	++samplesGotten;

	while (!eeprom_getNext(&packet2)) { ++samplesGotten; }

	// validate
	if (samplesGotten == testsize) {
		System_printf("%d samples gotten! Correct!\n", testsize);
	} else {
		System_printf("ERR: failure to retrieve samples using getNext(): %d\n", samplesGotten);
	}
}

void eeprom_testGetNextWithWrap() {
	System_printf("[eeprom_testGetNextWithWrap]\n");
	System_flush();

	unsigned testsize = 5;

	// test packet
	struct sensorPacket packet;
	packet.header.sourceAddress = 1;
	packet.header.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	packet.sampledata.tempData.timestamp = 0x12345678;
	packet.sampledata.tempData.temp_h = 0x5678;
	packet.sampledata.tempData.temp_l = 0x8765;
	packet.sampledata.heartRateData.rate_h = 0x7890;
	packet.sampledata.heartRateData.rate_l = 0x0987;
	packet.sampledata.heartRateData.temp_h = 0x2345;
	packet.sampledata.heartRateData.temp_l = 0x5432;

	eeprom_reset();

	// write thrice
	unsigned iter = 0;
	while (iter < testsize) {
		eeprom_write(&packet);
		++iter;
	}

	// samples retieved from mem
	unsigned samplesGotten = 0;


	// get from sample set, and loop until no samples left
	struct sensorPacket packet2;
	eeprom_getNext(&packet2);
	++samplesGotten;

	while (!eeprom_getNext(&packet2)) { ++samplesGotten; }

	// validate
	if (samplesGotten == MAX_EEPROM_ADDRESS / SAMPLE_SIZE) {
		System_printf("%d eeprom samples written/read!\n", MAX_EEPROM_ADDRESS / SAMPLE_SIZE);
	} else {
		System_printf("ERR: failure to retrieve samples using getNextWithWrap(): %d\n", samplesGotten);
	}
}

// Pre-condition:  eeprom address set to 0x0000
// Expects:        18 bytes written and read from eeprom
// Post-condition: 18 bytes stored in eeprom
void eeprom_testWriteReadSample() {
	System_printf("[eeprom_testWriteReadSample]\n");
	System_flush();

	// test packet
	struct sensorPacket packet;
	packet.header.sourceAddress = 1;
	packet.header.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	packet.sampledata.tempData.timestamp = 0x12345678;
	packet.sampledata.tempData.temp_h = 0x5678;
	packet.sampledata.tempData.temp_l = 0x8765;
	packet.sampledata.heartRateData.rate_h = 0x7890;
	packet.sampledata.heartRateData.rate_l = 0x0987;
	packet.sampledata.heartRateData.temp_h = 0x2345;
	packet.sampledata.heartRateData.temp_l = 0x5432;

	eeprom_reset();
	eeprom_write(&packet);
	eeprom_reset();

	// recreate packet from serialized bytes
	struct sensorPacket packet2;
	eeprom_getNext(&packet2);

	// verify unserialized packet
	bool success = packet.header.sourceAddress == packet2.header.sourceAddress;
	success = packet.header.packetType == packet2.header.packetType;
	success = packet.sampledata.tempData.timestamp = packet2.sampledata.tempData.timestamp;
	success = packet.sampledata.tempData.temp_h = packet2.sampledata.tempData.temp_h;
	success = packet.sampledata.tempData.temp_l = packet2.sampledata.tempData.temp_l;
	success = packet.sampledata.heartRateData.rate_h = packet2.sampledata.heartRateData.rate_h;
	success = packet.sampledata.heartRateData.rate_l = packet2.sampledata.heartRateData.rate_l;
	success = packet.sampledata.heartRateData.temp_h = packet2.sampledata.heartRateData.temp_h;
	success = packet.sampledata.heartRateData.temp_l = packet2.sampledata.heartRateData.temp_l;

	if (success) {
		System_printf("eeprom sample successfully written/read\n");
	} else {
		System_printf("ERR: eeprom sample failed to write/read!\n");
	}
}

// Pre-condition:  eeprom address set to 0x0000
// Expects:        all eeprom addresses written and read
// Post-condition: eeprom full of test data
void eeprom_testValidateMemory() {
	System_printf("[eeprom_testValidateMemory]\n");
	System_flush();

	// test settings
	uint16_t testsize = MAX_EEPROM_ADDRESS / SAMPLE_SIZE;

	// tally bad bytes
	unsigned badWrites = 0;
	unsigned badReads = 0;

	// test packet
	struct sensorPacket packet;
	packet.header.sourceAddress = 1;
	packet.header.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	packet.sampledata.tempData.timestamp = 0x12345678;
	packet.sampledata.tempData.temp_h = 0x5678;
	packet.sampledata.tempData.temp_l = 0x8765;
	packet.sampledata.heartRateData.rate_h = 0x7890;
	packet.sampledata.heartRateData.rate_l = 0x0987;
	packet.sampledata.heartRateData.temp_h = 0x2345;
	packet.sampledata.heartRateData.temp_l = 0x5432;

	// reset memory pointer
	eeprom_reset();

	// check writes
	for (; eeprom_currentAddress < testsize;) {
		if (!eeprom_write(&packet)) {
			badWrites++;
		}
	}

	// read packet
	struct sensorPacket packet2;
	packet2.header.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;

	// check reads
	for (; eeprom_currentAddress < testsize;) {
		eeprom_getNext(&packet2);

		// verify unserialized packet
		bool success = packet.header.sourceAddress == packet2.header.sourceAddress;
		success = packet.header.packetType == packet2.header.packetType;
		success = packet.sampledata.tempData.timestamp = packet2.sampledata.tempData.timestamp;
		success = packet.sampledata.tempData.temp_h = packet2.sampledata.tempData.temp_h;
		success = packet.sampledata.tempData.temp_l = packet2.sampledata.tempData.temp_l;
		success = packet.sampledata.heartRateData.rate_h = packet2.sampledata.heartRateData.rate_h;
		success = packet.sampledata.heartRateData.rate_l = packet2.sampledata.heartRateData.rate_l;
		success = packet.sampledata.heartRateData.temp_h = packet2.sampledata.heartRateData.temp_h;
		success = packet.sampledata.heartRateData.temp_l = packet2.sampledata.heartRateData.temp_l;

		if (!success) {
			++badReads;
		}
	}

	// count successful writes/reads
	System_printf("Good Writes = %d / %d\n", testsize - badWrites, testsize);
	System_printf("Good Reads = %d / %d\n", testsize - badReads, testsize);
	System_flush();
}

void eeprom_printSample(uint8_t *buf) {
	int i;
	for (i = 0; i < SAMPLE_SIZE; ++i) {
		System_printf("0x%02x ", buf[i]);
	}
	System_printf("\n");
}

#endif /* EEPROM_TEST_H_ */
