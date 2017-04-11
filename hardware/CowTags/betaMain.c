 /* main.c
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

/***** Includes *****/
#include "global_cfg.h"
#include <RadioSend.h>
/* XDCtools Header files */
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>

/* Board Header files */
#include <Board.h>
#include <pinTable.h>

/*test suites*/
#include <alphaRadioTest.h>
#include <betaRadioTest.h>
#include <eepromTest.h>
#include <serializeTest.h>
#include <arduinoComTest.h>
#include <xdc/runtime/Timestamp.h>
#include <EventManager.h>
#include "bootTimestamp.h"
#include <xdc/runtime/Types.h>

/* Global PIN_Config table */
PIN_State ledPinState;
PIN_Handle ledPinHandle;

int main(void) {
	// boot_timestamp is in bootTimestamp.h
	Types_FreqHz frequency;
	Timestamp_getFreq(&frequency);
	boot_timestamp = Timestamp_get32() / (frequency.hi << 8 | frequency.lo);

	if (verbose_main) {System_printf("Initializing tasks & debug file...\n");}

	//if (verbose_main) {System_printf("Initializing board...\n");}
	Board_initGeneral(); // init board

	//if (verbose_main) {System_printf("Initializing event manager...\n");}
	eventManager_init();

	//if (verbose_main) {System_printf("Initializing sensors...\n");}
	//Sensors_test(); // init i2C

	//if (verbose_main) {System_printf("Initializing EEPROM...\n");}
	//eepromTest_init();

//	if (verbose_main) {System_printf("Initializing serialization thread...\n");}
//	serializeTestStart();

	if(verbose_main){System_printf("Initializing radio antenna...\n");}
	radioSend_init();

	if(verbose_main){System_printf("Initializing Beta tasks...\n");}
	betaRadioTest_init();

	System_flush();

	if (verbose_main) {
		/* Open LED pins */
		ledPinHandle = PIN_open(&ledPinState, ledPinTable);

		if (!ledPinHandle) {
			System_printf("led pin table error code %i \n", ledPinHandle);
			System_flush();
			System_abort("Error initializing board LED pins\n");
			//(already allocated pin in a PinList or non-existent pin in aPinList)
		}

		PIN_setOutputValue(ledPinHandle, Board_LED1, 1); //signal init success
	}

	if (verbose_main) {
		System_printf("Starting BIOS:\n");
		System_flush();
	}

	BIOS_start();

	return (0);
}
