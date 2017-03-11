/*
 * main.c
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

/***** Includes *****/
#include <debug.h>

/* XDCtools Header files */
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>

/* Board Header files */
#include <Board.h>
#include <pinTable.h>

/*test suites*/
#include <alphaRadioTest.h>
#include <eepromTest.h>
#include <serializeTest.h>
#include <arduinoComTest.h>
#include <radioSendReceive.h>
#include <EventManager.h>

/* Global PIN_Config table */
PIN_State ledPinState;
PIN_Handle ledPinHandle;
/*******************************************/

int main(void){
	if(verbose_main){System_printf("Initializing tasks...\n");}

	if(verbose_main){System_printf("Initializing board...\n");}
	Board_initGeneral(); // init board

	eventManager_init();

	//if(verbose_main){System_printf("Initializing sensors...\n");}
	//Sensors_init(); // init i2C

	//if(verbose_main){System_printf("Initializing EEPROM...\n");}
	//eeprom_testStart();

	//if(verbose_main){System_printf("Initializing serialization thread...\n");}
	//serialize_testStart();

	if(verbose_main){System_printf("Initializing radio antenna...\n");}
	radioSendReceive_init();

	if(verbose_main){System_printf("Initializing alpha tasks...\n");}
	alphaRadioTest_init();

	//if(verbose_main){System_printf("Initializing Arduino communication...\n");}
	//arduinoTest_init();

	System_flush();

	if(verbose_main){
	/* Open LED pins */
	ledPinHandle = PIN_open(&ledPinState, ledPinTable);
	if(!ledPinHandle) {
		System_printf("led pin table error code %i \n", ledPinHandle);
		System_flush();
		System_abort("Error initializing board LED pins\n");
		//(already allocated pin in a PinList or non-existent pin in aPinList)
	}
	PIN_setOutputValue(ledPinHandle, Board_LED1, 1); //signal init success
	}

	if(verbose_main){
		System_printf("Starting BIOS:\n");
		System_flush();
	}

	BIOS_start();

	return (0);
}
