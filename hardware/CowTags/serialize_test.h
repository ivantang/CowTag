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
void serialize_testSerializePacket();

bool serialize_testStart() {
	// start eeprom thread
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)serialize_testSerializePacket, &taskParams, NULL);

	return true;
}

void serialize_testSerializePacket() {
	System_printf("[serialize_testSerialPacket]\n");

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

	uint8_t buffer[18];
	serializePacket(&packet, buffer);

	struct sensorPacket packet2;
	packet2.header.packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	unserializePacket(&packet2, buffer);

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
		System_printf("packet successfully un/serialized\n");
	} else {
		System_printf("ERR: packet failed to un/serialize!\n");
	}
}

#endif /* SERIALIZE_TEST_H_ */
