/*
 * main.c
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

/* XDCtools Header files */
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
#include "i2c.h"
#include "radioProtocol.h"
#include "betaTask.h"
#include "betaRadio.h"
#include "pinTable.h"

/*******************************************/
/**constants*/
#define TASKSTACKSIZE		1024	//i2c

/**registers*/
#define	LIS3DH_TEMP			0x07  //i2c accelerometer temp result register

/**structures*/

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/* Global PIN_Config table */
PIN_State ledPinState;
PIN_Handle ledPinHandle;
/*******************************************/

int main(void)
{
    Task_Params taskParams;

	System_printf("Initializing tasks...\n");
	System_flush();

    Board_initGeneral(); // init board
    //Board_initI2C(); // init i2C

    BetaRadio_init(); // init rf beta
    BetaTask_init(); // init beta

    /* Construct BIOS objects */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    //Task_construct(&task0Struct, (Task_FuncPtr)initLIS3DH, &taskParams, NULL);

    /* Open LED pins before BIOS*/
    //Task_construct(&task0Struct, (Task_FuncPtr)initLIS3DH, &taskParams, NULL);
    //Task_construct(&task0Struct, (Task_FuncPtr)initMIKROE1362, &taskParams, NULL);
    //Task_construct(&task0Struct, (Task_FuncPtr)initMAX30100, &taskParams, NULL);
    Task_construct(&task0Struct, (Task_FuncPtr)initSensors, &taskParams, NULL);
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











