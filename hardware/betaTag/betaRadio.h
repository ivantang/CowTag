/*
 * betaRadio.h
 *
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef TASKS_BETARADIO_H_
#define TASKS_BETARADIO_H_

#include "stdint.h"

enum NodeRadioOperationStatus {
    NodeRadioStatus_Success,
    NodeRadioStatus_Failed,
    NodeRadioStatus_FailedNotConnected,
};

/* Initializes the NodeRadioTask and creates all TI-RTOS objects */
void BetaRadio_init(void);

/* Sends an ADC value to the concentrator */
enum NodeRadioOperationStatus NodeRadioTask_sendAdcData(uint16_t data);


#endif /* TASKS_BETARADIO_H_ */
