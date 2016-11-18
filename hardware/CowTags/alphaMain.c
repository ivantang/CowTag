/*
 * main.c
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

/* XDCtools Header files */
#include <alphaRadioTest.h>
#include <betaRadioTest.h>
#include <RadioSend.h>
#include <sensors.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h> //i2c
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/Power.h> //rf
#include <ti/drivers/power/PowerCC26XX.h> //rf

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>

/*standard libraries*/
#include <stdint.h>

/*custom headers*/
#include "Board.h"
#include "radioProtocol.h"
#include "pinTable.h"
#include "RadioReceive.h"

/* Global PIN_Config table */
PIN_State ledPinState;
PIN_Handle ledPinHandle;
/*******************************************/

int main(void)
{

	System_printf("Initializing tasks...\n");
	System_flush();

	Board_initGeneral(); // init board
	//Sensors_init(); // init i2C

	radioReceive_init();
	alphaRadioTest_init();

	/* Open LED pins */
	ledPinHandle = PIN_open(&ledPinState, ledPinTable);
	if(!ledPinHandle) {
		System_printf("led pin table error code %i \n", ledPinHandle);
		System_flush();
		System_abort("Error initializing board LED pins\n");
		//(already allocated pin in a PinList or non-existent pin in aPinList)
	}
	PIN_setOutputValue(ledPinHandle, Board_LED1, 1); //signal init success


	System_printf("Starting BIOS:\n");
	System_flush();
	BIOS_start();

	return (0);
}
