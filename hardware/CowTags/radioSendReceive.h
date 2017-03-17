/*
 * radioSendReceive.h
 *
 *  Created on: Mar 2, 2017
 *      Author: annik
 */

#ifndef RADIOSENDRECEIVE_H_
#define RADIOSENDRECEIVE_H_


#include <radioProtocol.h>
#include <stdint.h>

enum alphaRadioOperationStatus {
	AlphaRadioStatus_Success,
	AlphaRadioStatus_Failed,
	AlphaRadioStatus_FailedNotConnected,
	AlphaRadioStatus_ReceivedValidPacket
};

union ConcentratorPacket {
    struct PacketHeader header;
    struct sensorPacket sensorPacket;
};

/* Initializes the NodeRadioTask and creates all TI-RTOS objects */
void radioSendReceive_init(void);

typedef void (*ConcentratorRadio_PacketReceivedCallback)(union ConcentratorPacket* packet, int8_t rssi);

/* Register the packet received callback */
void AlphaRadioTask_registerPacketReceivedCallback(ConcentratorRadio_PacketReceivedCallback callback);

/* receiving function */
enum alphaRadioOperationStatus alphaRadioReceiveData(void);

/* sending function */
enum alphaRadioOperationStatus alphaRadioSendData(struct sampleData data);



#endif /* RADIOSENDRECEIVE_H_ */
