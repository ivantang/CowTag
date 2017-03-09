/*
 * betaRadio.h
 *
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef RADIOSEND_H_
#define RADIOSEND_H_

#include <radioProtocol.h>

enum NodeRadioOperationStatus {
    NodeRadioStatus_Success,
    NodeRadioStatus_Failed,
    NodeRadioStatus_FailedNotConnected,
};

/* Initializes the NodeRadioTask and creates all TI-RTOS objects */
void radioSend_init(void);

/* Sends an ADC value to the concentrator */
enum NodeRadioOperationStatus betaRadioSendData(struct sampleData data);

#endif /* TASKS_BETARADIO_H_ */
