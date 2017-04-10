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
void serializeTestSerializeNormal();
void serializeTestSerializeAccelerometer();

bool serializeTestStart() {
	// start test thread
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)serializeTestSerializeAccelerometer, &taskParams, NULL);

	return true;
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
	sampledata.cowID = 1;
	sampledata.packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
	sampledata.timestamp = 0x12345678;
	sampledata.accelerometerData.x_h = 0x78;
	sampledata.accelerometerData.x_l = 0x78;
	sampledata.accelerometerData.y_h = 0x89;
	sampledata.accelerometerData.y_l = 0x89;
	sampledata.accelerometerData.z_h = 0x90;
	sampledata.accelerometerData.z_l = 0x90;

	sampledata.error = 0x0;


	uint8_t buffer[SAMPLE_SIZE];
	serializePacket(&sampledata, buffer);

	struct sampleData sample2;
	unserializePacket(&sample2, buffer);

	// verify unserialized packet
	bool success = (sampledata.cowID == sample2.cowID);
	success = (sampledata.packetType == sample2.packetType);
	success = (sampledata.timestamp == sample2.timestamp);
	success = (sampledata.accelerometerData.x_h == sample2.accelerometerData.x_h);
	success = (sampledata.accelerometerData.x_l == sample2.accelerometerData.x_l);
	success = (sampledata.accelerometerData.y_h == sample2.accelerometerData.y_h);
	success = (sampledata.accelerometerData.y_l == sample2.accelerometerData.y_l);
	success = (sampledata.accelerometerData.z_h == sample2.accelerometerData.z_h);
	success = (sampledata.accelerometerData.z_l == sample2.accelerometerData.z_l);

	success = (sampledata.error == sample2.error);

	if (success) {
		if(verbose_serializeTest){System_printf("packet successfully un/serialized\n");}
	} else {
		if(verbose_serializeTest){System_printf("ERR: packet failed to un/serialize!\n");}
	}
}

#endif /* SERIALIZETEST_H_ */
