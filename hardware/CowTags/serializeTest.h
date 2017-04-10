/*
 * serialize_test.h
 *
 *  Created on: Feb 6, 2017
 *      Author: champ
 */

#ifndef SERIALIZETEST_H_
#define SERIALIZETEST_H_

#include <radioProtocol.h>
#include <serialize.h>

/* XDCtools Header files */
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/*constants*/
#define TASKSTACKSIZE 1024 //i2c

/***** structures *****/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/* test prototypes */
bool serializeTestStart();
void serializeTestRunner();
void serializeTestSerializeNormal();
void serializeTestSerializeAccelerometer();

bool serializeTestStart() {
	// start test thread
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)serializeTestRunner, &taskParams, NULL);

	return true;
}

void serializeTestRunner() {
	serializeTestSerializeNormal();
	serializeTestSerializeAccelerometer();
}

// create fake Normal sample data, then read and write from eeprom. Confirm data is retrieved correctly
void serializeTestSerializeNormal() {
	if(verbose_serializeTest){System_printf("[serializeTestSerializeNormal]\n");}

	// test packet
	struct sampleData sampledata;
	sampledata.cowID                = 1;
	sampledata.packetType           = RADIO_PACKET_TYPE_SENSOR_PACKET;
	sampledata.timestamp            = 0x12345678;
	sampledata.tempData.temp_h      = 0x78;
	sampledata.tempData.temp_l      = 0x65;
	sampledata.heartRateData.rate_h = 0x90;
	sampledata.heartRateData.rate_l = 0x87;
	sampledata.heartRateData.temp_h = 0x45;
	sampledata.heartRateData.temp_l = 0x32;
	sampledata.error                = 0x0;

	uint8_t buffer[SAMPLE_SIZE];
	serializePacket(&sampledata, buffer);

	struct sampleData sample2;
	unserializePacket(&sample2, buffer);

	// verify unserialized packet
	bool success = (sampledata.cowID == sample2.cowID)
		&& (sampledata.packetType == sample2.packetType)
		&& (sampledata.timestamp == sample2.timestamp)
		&& (sampledata.tempData.temp_h == sample2.tempData.temp_h)
		&& (sampledata.tempData.temp_l == sample2.tempData.temp_l)
		&& (sampledata.heartRateData.rate_h == sample2.heartRateData.rate_h)
		&& (sampledata.heartRateData.rate_l == sample2.heartRateData.rate_l)
		&& (sampledata.heartRateData.temp_h == sample2.heartRateData.temp_h)
		&& (sampledata.heartRateData.temp_l == sample2.heartRateData.temp_l)
		&& (sampledata.error == sample2.error);

	if (success) {
		if(verbose_serializeTest){System_printf("packet successfully unserialized\n");}
	} else {
		if(verbose_serializeTest){System_printf("ERR: packet failed to unserialize!\n");}
	}
}

// create fake Accelerometer sample data, then read and write from eeprom. Confirm data is retrieved correctly
void serializeTestSerializeAccelerometer() {
	if(verbose_serializeTest){System_printf("[serializeTestSerializeAccelerometer]\n");}

	// test packet
	struct sampleData sampledata;
	sampledata.cowID               = 1;
	sampledata.packetType          = RADIO_PACKET_TYPE_ACCEL_PACKET;
	sampledata.timestamp           = 0x12345678;
	sampledata.accelerometerData.x = 0x78;
	sampledata.accelerometerData.y = 0x89;
	sampledata.accelerometerData.z = 0x90;
	sampledata.error               = 0x0;


	uint8_t buffer[SAMPLE_SIZE];
	serializePacket(&sampledata, buffer);

	struct sampleData sample2;
	unserializePacket(&sample2, buffer);

	// verify unserialized packet
	bool success = (sampledata.cowID == sample2.cowID)
		&& (sampledata.packetType == sample2.packetType)
		&& (sampledata.timestamp == sample2.timestamp)
		&& (sampledata.accelerometerData.x == sample2.accelerometerData.x)
		&& (sampledata.accelerometerData.y == sample2.accelerometerData.y)
		&& (sampledata.accelerometerData.z == sample2.accelerometerData.z)
		&& (sampledata.error == sample2.error);

	if (success) {
		if(verbose_serializeTest){System_printf("packet successfully unserialized\n");}
	} else {
		if(verbose_serializeTest){System_printf("ERR: packet failed to unserialize!\n");}
	}
}

#endif /* SERIALIZETEST_H_ */
