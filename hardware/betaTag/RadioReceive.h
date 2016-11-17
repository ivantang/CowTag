/*
 * RadioReceive.h
 *
 *  Created on: Nov 17, 2016
 *      Author: annik
 */

#ifndef RADIORECEIVE_H_
#define RADIORECEIVE_H_

// temporary
// to be combined into beta radio

#include "stdint.h"
#include "radioProtocol.h"

enum ConcentratorRadioOperationStatus {
    ConcentratorRadioStatus_Success,
    ConcentratorRadioStatus_Failed,
    ConcentratorRadioStatus_FailedNotConnected,
};


union ConcentratorPacket {
    struct PacketHeader header;
    struct BetaPacket betaPacket;
    struct DualModeSensorPacket dmSensorPacket;
};

typedef void (*ConcentratorRadio_PacketReceivedCallback)(union ConcentratorPacket* packet, int8_t rssi);

/* Create the ConcentratorRadioTask and creates all TI-RTOS objects */
void RadioReceive_init(void);

/* Register the packet received callback */
void ConcentratorRadioTask_registerPacketReceivedCallback(ConcentratorRadio_PacketReceivedCallback callback);


#endif /* RADIORECEIVE_H_ */
