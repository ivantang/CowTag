/*
 * serialize.c
 *
 *  Created on: Feb 6, 2017
 *      Author: champ
 */
#include <debug.h>
#include <xdc/runtime/System.h>
#include <radioProtocol.h>
#include <serialize.h>

void serializePacket(struct sampleData *data, uint8_t *buffer) {
	buffer[0] = data->cowID;
	buffer[1] = data->packetType;
	buffer[2] = data->timestamp >> 24 & 0xff;
	buffer[3] = data->timestamp >> 16 & 0xff;
	buffer[4] = data->timestamp >> 8 & 0xff;
	buffer[5] = data->timestamp & 0xff;

	if (data->packetType == RADIO_PACKET_TYPE_SENSOR_PACKET) {
		buffer[6] = data->tempData.temp_h;
		buffer[7] = data->tempData.temp_l;
		buffer[8] = data->heartRateData.rate_h;
		buffer[9] = data->heartRateData.rate_l;
		buffer[10] = data->heartRateData.temp_h;
		buffer[11] = data->heartRateData.temp_l;
		buffer[12] = data->error = 0;

	} else if (data->packetType == RADIO_PACKET_TYPE_ACCEL_PACKET) {
		buffer[6] = data->accelerometerData.x;
		buffer[7] = data->accelerometerData.y;
		buffer[8] = data->accelerometerData.z;
		buffer[9] = data->error = 0;

	} else {
		if(verbose_serialize){System_printf("ERR: unrecognized packet type when serializing: %d\n", data->cowID);}
	}
}

void unserializePacket(struct sampleData *data, uint8_t *buffer) {
	data->cowID = buffer[0];
	data->packetType = buffer[1];
	data->timestamp =
			(buffer[2] << 24) |
			(buffer[3] << 16) |
			(buffer[4] << 8) |
			buffer[5];

	if (data->packetType == RADIO_PACKET_TYPE_SENSOR_PACKET) {
		data->tempData.temp_h = buffer[6];
		data->tempData.temp_l = buffer[7];
		data->heartRateData.rate_h = buffer[8];
		data->heartRateData.rate_l = buffer[9];
		data->heartRateData.temp_h = buffer[10];
		data->heartRateData.temp_l = buffer[11];
		data->error = buffer[12];

	} else if (data->packetType == RADIO_PACKET_TYPE_ACCEL_PACKET) {
		data->accelerometerData.x = buffer[6];
		data->accelerometerData.y = buffer[7];
		data->accelerometerData.z = buffer[8];
		data->error = buffer[9];

	} else {
		if(verbose_serialize){System_printf("ERR: unrecognized packet type when unserializing: %d\n", data->cowID);}
	}
}
