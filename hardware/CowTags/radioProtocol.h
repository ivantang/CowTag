/*
 * radioProtocol.h
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef RADIOPROTOCOL_H_
#define RADIOPROTOCOL_H_

#include "stdint.h"
#include <sensors.h>
#include "easylink/EasyLink.h"

#define RADIO_CONCENTRATOR_ADDRESS     0x00
#define RADIO_NODE_ADDRESS				0x10
#define RADIO_EASYLINK_MODULATION     EasyLink_Phy_Custom

#define RADIO_PACKET_TYPE_ACK_PACKET             0
#define RADIO_PACKET_TYPE_ADC_SENSOR_PACKET      1
#define RADIO_PACKET_TYPE_DM_SENSOR_PACKET       2
#define RADIO_PACKET_TYPE_SENSOR_PACKET    		 3

struct PacketHeader {
    uint8_t sourceAddress;
    uint8_t packetType;
    uint8_t error;
};

/* will be concat of data from all sensors */
struct sampleData {
	struct temperatureData tempData;
	struct accelerationData accelerometerData;
	struct heartrateData heartRateData;
};

/* our custom beta packet with sensor info */
struct sensorPacket{
	struct PacketHeader header;
	struct sampleData sampledata;
};

struct AckPacket {
    struct PacketHeader header;
};

#endif /* RADIOPROTOCOL_H_ */
