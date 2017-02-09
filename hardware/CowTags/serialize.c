/*
 * serialize.c
 *
 *  Created on: Feb 6, 2017
 *      Author: champ
 */

#include <stdint.h>
#include <stdbool.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include "serialize.h"


void serializePacket(struct sampleData *data, uint8_t *buffer) {
	if (data->packetType == RADIO_PACKET_TYPE_SENSOR_PACKET) {
		buffer[0] = data->cowID;
		buffer[1] = data->packetType;
		buffer[2] = data->tempData.timestamp >> 24 & 0xff;
		buffer[3] = data->tempData.timestamp >> 16 & 0xff;
		buffer[4] = data->tempData.timestamp >> 8 & 0xff;
		buffer[5] = data->tempData.timestamp & 0xff;
		buffer[6] = data->tempData.temp_h >> 8 & 0xff;
		buffer[7] = data->tempData.temp_h & 0xff;
		buffer[8] = data->tempData.temp_l >> 8 & 0xff;
		buffer[9] = data->tempData.temp_l & 0xff;
		buffer[10] = data->heartRateData.rate_h >> 8 & 0xff;
		buffer[11] = data->heartRateData.rate_h & 0xff;
		buffer[12] = data->heartRateData.rate_l >> 8 & 0xff;
		buffer[13] = data->heartRateData.rate_l & 0xff;
		buffer[14] = data->heartRateData.temp_h >> 8 & 0xff;
		buffer[15] = data->heartRateData.temp_h & 0xff;
		buffer[16] = data->heartRateData.temp_l >> 8 & 0xff;
		buffer[17] = data->heartRateData.temp_l & 0xff;
		buffer[18] = data->error = 0;
	} else {
		System_printf("ERR: unrecognized packet type when serializing: %d\n", data->cowID);
	}
}

//uint8_t sourceAddress;  // sample source id
//uint8_t packetType;
//uint8_t error;
//uint32_t timestamp;
//struct temperatureData tempData;
//struct accelerationData accelerometerData;
//struct heartrateData heartRateData;

void unserializePacket(struct sampleData *data, uint8_t *buffer) {
	if (data->packetType == RADIO_PACKET_TYPE_SENSOR_PACKET) {
		data->cowID = buffer[0];
		data->packetType = buffer[1];
		data->tempData.timestamp =
				(buffer[2] << 24) |
				(buffer[3] << 16) |
				(buffer[4] << 8) |
				buffer[5];

		data->tempData.temp_h = (buffer[6] << 8) | buffer[7];
		data->tempData.temp_l = (buffer[8] << 8) | buffer[9];
		data->heartRateData.rate_h = (buffer[10] << 8) | buffer[11];
		data->heartRateData.rate_l = (buffer[12] << 8) | buffer[13];
		data->heartRateData.temp_h = (buffer[14] << 8) | buffer[15];
		data->heartRateData.temp_l = (buffer[16] << 8) | buffer[17];
		data->error = buffer[18];
	} else {
		System_printf("ERR: unrecognized packet type when unserializing: %d\n", data->cowID);
	}
}
