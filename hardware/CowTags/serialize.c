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


void serializePacket(struct sensorPacket *packet, uint8_t *buffer) {
	buffer[0] = packet->header.sourceAddress;
	buffer[1] = packet->header.packetType;

	if (packet->header.packetType == RADIO_PACKET_TYPE_ACK_PACKET) {

	} else if (packet->header.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET) {
		buffer[2] = packet->sampledata.tempData.timestamp >> 24 & 0xff;
		buffer[3] = packet->sampledata.tempData.timestamp >> 16 & 0xff;
		buffer[4] = packet->sampledata.tempData.timestamp >> 8 & 0xff;
		buffer[5] = packet->sampledata.tempData.timestamp & 0xff;
		buffer[6] = packet->sampledata.tempData.temp_h >> 8 & 0xff;
		buffer[7] = packet->sampledata.tempData.temp_h & 0xff;
		buffer[8] = packet->sampledata.tempData.temp_l >> 8 & 0xff;
		buffer[9] = packet->sampledata.tempData.temp_l & 0xff;
		buffer[10] = packet->sampledata.heartRateData.rate_h >> 8 & 0xff;
		buffer[11] = packet->sampledata.heartRateData.rate_h & 0xff;
		buffer[12] = packet->sampledata.heartRateData.rate_l >> 8 & 0xff;
		buffer[13] = packet->sampledata.heartRateData.rate_l & 0xff;
		buffer[14] = packet->sampledata.heartRateData.temp_h >> 8 & 0xff;
		buffer[15] = packet->sampledata.heartRateData.temp_h & 0xff;
		buffer[16] = packet->sampledata.heartRateData.temp_l >> 8 & 0xff;
		buffer[17] = packet->sampledata.heartRateData.temp_l & 0xff;
	} else {
		System_printf("ERR: unrecognized packet type when serializing: %d\n", packet->header.sourceAddress);
	}
}

void unserializePacket(struct sensorPacket *packet, uint8_t *buffer) {
	packet->header.sourceAddress = buffer[0];
	packet->header.packetType = buffer[1];

	if (packet->header.packetType == RADIO_PACKET_TYPE_SENSOR_PACKET) {
		packet->sampledata.tempData.timestamp =
				(buffer[2] << 24) |
				(buffer[3] << 16) |
				(buffer[4] << 8) |
				buffer[5];

		packet->sampledata.tempData.temp_h = (buffer[6] << 8) | buffer[7];
		packet->sampledata.tempData.temp_l = (buffer[8] << 8) | buffer[9];
		packet->sampledata.heartRateData.rate_h = (buffer[10] << 8) | buffer[11];
		packet->sampledata.heartRateData.rate_l = (buffer[12] << 8) | buffer[13];
		packet->sampledata.heartRateData.temp_h = (buffer[14] << 8) | buffer[15];
		packet->sampledata.heartRateData.temp_l = (buffer[16] << 8) | buffer[17];
	} else {
		System_printf("ERR: unrecognized packet type when unserializing: %d\n", packet->header.sourceAddress);
	}
}
