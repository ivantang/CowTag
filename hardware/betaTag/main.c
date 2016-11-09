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
#include <ti/drivers/power/PowerCC26XX.h> //rf //(TODO: do we need to change to power cc13xx?)

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

/*******************************************/
/**constants*/
#define TASKSTACKSIZE		1024	//i2c

/**registers*/
#define	LIS3DH_TEMP			0x07  //i2c accelerometer temp result register

/**structures*/

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/* Global memory storage for a PIN_Config table */
static PIN_State ledPinState;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config ledPinTable[] = {
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

/*******************************************/

int main(void)
{
    PIN_Handle ledPinHandle;
    Task_Params taskParams;


	System_printf("Initializing tasks...\n");
	System_flush();

    Board_initGeneral(); // init board
    Board_initI2C(); // init i2C

    /* Initialize rf sensor node tasks */
    BetaRadio_init();
    BetaTask_init();

    /* Construct BIOS objects */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    //Task_construct(&task0Struct, (Task_FuncPtr)initLIS3DH, &taskParams, NULL);
    Task_construct(&task0Struct, (Task_FuncPtr)initMIKROE1362, &taskParams, NULL);
    /* Open LED pins */
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if(!ledPinHandle) {
        System_abort("Error initializing board LED pins\n");
    }

    PIN_setOutputValue(ledPinHandle, Board_LED1, 1);


    System_printf("Starting BIOS:\n"
    			  "System provider is set to SysMin. \n"
    			  "Halt the target to view any SysMin contents in "
                  "ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}











