/*
 * serialize_test.h
 *
 *  Created on: Feb 6, 2017
 *      Author: champ
 */

#ifndef SERIALIZE_TEST_H_
#define SERIALIZE_TEST_H_

#include "radioProtocol.h"
#include "serialize.h"

/*constants*/
#define TASKSTACKSIZE		1024	//i2c


/* test prototypes */
bool serialize_testStart();
void serialize_testSerializeNormal();
void serialize_testSerializeAccelerometer();

bool serialize_testStart() {
	// start test thread
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)serialize_testSerializeAccelerometer, &taskParams, NULL);

	return true;
}

void serialize_testSerializeNormal() {
	System_printf("[serialize_testSerializeNormal]\n");

	// test packet
	struct sampleData sampledata;
	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.tempData.temp_h = 0x5678;
	sampledata.tempData.temp_l = 0x8765;
	sampledata.heartRateData.rate_h = 0x7890;
	sampledata.heartRateData.rate_l = 0x0987;
	sampledata.heartRateData.temp_h = 0x2345;
	sampledata.heartRateData.temp_l = 0x5432;

	uint8_t buffer[SAMPLE_SIZE];
	serializePacket(&sampledata, buffer);

	struct sampleData sample2;
	unserializePacket(&sample2, buffer);

	// verify unserialized packet
	bool success = sampledata.cowID == sample2.cowID;
	success = sampledata.packetType == sample2.packetType;
	success = sampledata.tempData.timestamp = sample2.tempData.timestamp;
	success = sampledata.tempData.temp_h = sample2.tempData.temp_h;
	success = sampledata.tempData.temp_l = sample2.tempData.temp_l;
	success = sampledata.heartRateData.rate_h = sample2.heartRateData.rate_h;
	success = sampledata.heartRateData.rate_l = sample2.heartRateData.rate_l;
	success = sampledata.heartRateData.temp_h = sample2.heartRateData.temp_h;
	success = sampledata.heartRateData.temp_l = sample2.heartRateData.temp_l;

	if (success) {
		System_printf("packet successfully un/serialized\n");
	} else {
		System_printf("ERR: packet failed to un/serialize!\n");
	}
}

void serialize_testSerializeAccelerometer() {
	System_printf("[serialize_testSerializeAccelerometer]\n");

	// test packet
	struct sampleData sampledata;
	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.accelerometerData.x = 0x5678;
	sampledata.accelerometerData.y = 0x6789;
	sampledata.accelerometerData.z = 0x7890;

	uint8_t buffer[SAMPLE_SIZE];
	serializePacket(&sampledata, buffer);

	struct sampleData sample2;
	unserializePacket(&sample2, buffer);

	// verify unserialized packet
	bool success = sampledata.cowID == sample2.cowID;
	success = sampledata.packetType == sample2.packetType;
	success = sampledata.tempData.timestamp = sample2.tempData.timestamp;
	sampledata.accelerometerData.x = sample2.accelerometerData.x;
	sampledata.accelerometerData.y = sample2.accelerometerData.y;
	sampledata.accelerometerData.z = sample2.accelerometerData.z;

	if (success) {
		System_printf("packet successfully un/serialized\n");
	} else {
		System_printf("ERR: packet failed to un/serialize!\n");
	}
}

#endif /* SERIALIZE_TEST_H_ */
