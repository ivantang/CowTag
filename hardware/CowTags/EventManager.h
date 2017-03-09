/*
 * EventManager.h
 *
 *  Created on: Mar 4, 2017
 *      Author: champ
 */

#ifndef EVENTMANAGER_H_
#define EVENTMANAGER_H_

#include <ti/sysbios/knl/Event.h>

#define RADIO_EVENT_ALL                 0xFFFFFFFF
#define RADIO_EVENT_SEND_DATA      		(uint32_t)(1 << 0)
#define RADIO_EVENT_DATA_ACK_RECEIVED   (uint32_t)(1 << 1)
#define RADIO_EVENT_ACK_TIMEOUT         (uint32_t)(1 << 2)
#define RADIO_EVENT_SEND_FAIL           (uint32_t)(1 << 3)


void eventManager_init();
Event_Handle * getEventHandle();

#endif /* EVENTMANAGER_H_ */
