/*
 * radioProtocol.h
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef RADIOPROTOCOL_H_
#define RADIOPROTOCOL_H_

#include "stdint.h"
#include "easylink/EasyLink.h"

#define RADIO_CONCENTRATOR_ADDRESS     0x00
#define RADIO_EASYLINK_MODULATION     EasyLink_Phy_Custom

#define RADIO_PACKET_TYPE_ACK_PACKET             0
#define RADIO_PACKET_TYPE_ADC_SENSOR_PACKET      1
#define RADIO_PACKET_TYPE_DM_SENSOR_PACKET       2
#define RADIO_PACKET_TYPE_BETA_PACKET    		 3

struct PacketHeader {
    uint8_t sourceAddress;
    uint8_t packetType;
};

/* will be concat of data from all sensors */
struct BetaData {
	uint16_t sensorData; // TODO: change to uint64_t?
};

/* our custom beta packet with sensor info */
struct BetaPacket{
	struct PacketHeader header;
	struct BetaData betadata;
};





struct AdcSensorPacket {
    struct PacketHeader header;
    uint16_t adcValue;
};

struct DualModeSensorPacket {
    struct PacketHeader header;
    uint16_t adcValue;
    uint16_t batt;
    uint32_t time100MiliSec;
    uint8_t button;
};

struct AckPacket {
    struct PacketHeader header;
};


#endif /* RADIOPROTOCOL_H_ */
