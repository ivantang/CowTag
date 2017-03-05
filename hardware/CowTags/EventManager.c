/*
 * EventManager.c
 *
 *  Created on: Mar 4, 2017
 *      Author: champ
 */

#include <ti/sysbios/knl/Event.h>

static Event_Handle masterEventHandle;
Event_Struct radioOperationEvent;

void eventManager_init()
{
	Event_Params eventParam;
	Event_Params_init(&eventParam);
	Event_construct(&radioOperationEvent, &eventParam);
	masterEventHandle = Event_handle(&radioOperationEvent);
}

Event_Handle * getEventHandle()
{
	return &masterEventHandle;
}
