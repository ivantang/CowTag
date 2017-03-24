/*
 * radioProtocol.h
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef RADIOPROTOCOL_H_
#define RADIOPROTOCOL_H_

#include <stdint.h>
#include <easylink/EasyLink.h>


#define RADIO_CONCENTRATOR_ADDRESS     0x00
//#define RADIO_NODE_ADDRESS				0x10

#define RADIO_EASYLINK_MODULATION     EasyLink_Phy_Custom

#define RADIO_PACKET_TYPE_ACK_PACKET             0
#define RADIO_PACKET_TYPE_SENSOR_PACKET    		 3
#define RADIO_PACKET_TYPE_ACCEL_PACKET           4
//delete RADIO_PACKET_TYPE_TEMP_PACKET afterwards, just using this for the farm demo :)
#define RADIO_PACKET_TYPE_TEMP_PACKET           5


#define SAMPLE_SIZE 13


// masks of types of addresses
#define BETA_ADDRESS 0x0
#define ALPHA_ADDRESS 0X1
#define GATEWAY_ADDRESS 0X3

struct PacketHeader {
    uint8_t sourceAddress;  // current hardware id
    uint8_t packetType;
};

struct temperatureData {
	uint8_t temp_l;
	uint8_t temp_h;
};

struct accelerationData {
	uint16_t x;
	uint16_t y;
	uint16_t z;
};

struct heartrateData {
	uint8_t temp_l;
	uint8_t temp_h;
	uint8_t rate_l;
	uint8_t rate_h;
};

/* will be concat of data from all sensors */
struct sampleData {
	uint8_t cowID;        // sample source id
	uint8_t packetType;
	uint32_t timestamp;
	struct temperatureData tempData;
	struct accelerationData accelerometerData;
	struct heartrateData heartRateData;
    uint8_t error;
};

/* our custom packet with sensor info */
struct sensorPacket{
	struct PacketHeader header;
	struct sampleData sampledata;
};

struct AckPacket {
    struct PacketHeader header;
};

#endif /* RADIOPROTOCOL_H_ */
