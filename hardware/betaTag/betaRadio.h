/*
 * betaRadio.h
 *
 * Beta is a node. Node is a Beta.
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef BETARADIO_H_
#define BETARADIO_H_

#include "stdint.h"

#define NODE_ACTIVITY_LED Board_LED0

enum BetaRadioStatus {
    RadioStatus_Success,
    RadioStatus_Failed,
    RadioStatus_FailedNotConnected,
};

/*******************************************/

/*initialize radio module*/
void BetaRadio_init(void );

/* Sends: a data value to the concentrator
 * returns: status */
enum BetaRadioStatus BetaRadio_sendData(uint16_t data);



#endif /* BETARADIO_H_ */
