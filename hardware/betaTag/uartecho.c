/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h> //i2c
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "Board.h"

#include <stdint.h>
#include <assert.h>

//#define TASKSTACKSIZE     768
#define TASKSTACKSIZE		1024	//i2c
#define	LIS3DH_TEMP			0x07  //i2c accelerometer temp result register

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

/*boolean to show if transfer is done*/
bool	transferDone = false;
/*boolean to show when ready to receive*/
bool	receiveStart = false;
/*
 *  ======== echoFxn ========
 *  Task for this function is created statically. See the project's .cfg file.
 */
Void echoFxn(UArg arg0, UArg arg1)
{
    char input;
    UART_Handle uart;
    UART_Params uartParams;
    const char echoPrompt[] = "Echoing characters:\r\n";
    //const char newLine[] = "\n";

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 9600;
    uart = UART_open(Board_UART0, &uartParams);

    if (uart == NULL) {
        System_abort("Error opening the UART");
    }

    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Loop forever echoing */
    while (1) {
        UART_read(uart, &input, 1);
        UART_write(uart, &input, 1);
    }
}

static void transferCallback(I2C_Handle handle, I2C_Transaction *transac, bool result)
{
    // Set length bytes
    if (result) {
        transferDone = true;
        System_printf("Transaction complete\n");
    } else {
        // Transaction failed, act accordingly...
        System_printf("Transaction failed\n");
    }
    System_flush();
}

//currently not using this function
//TODO: get this working
static void writeI2CRegister(uint8_t destination[], uint8_t value[]){
	unsigned int 	i;
	//uint8_t			txBuffer[sizeof(destination) + sizeof(value)];
    uint8_t			txBuffer[10];
	uint8_t         rxBuffer[2];
	I2C_Transaction t_i2cTransaction;
	I2C_Handle 		t_handle;
	I2C_Params		t_params;

	I2C_Params_init(&t_params);
    t_params.transferMode = I2C_MODE_CALLBACK;
    t_params.transferCallbackFxn = transferCallback;

    /*check if destination and value arrays are same size*/
    //if(sizeof(destination) != sizeof(value))
    assert(sizeof(destination) == sizeof(value));
    //System_printf("I2CTx: Size of destination array does not match size of value array\n");

    /*prepare data to send*/
    /*for(i = 0; i < sizeof(destination) + 1; i++){
    	txBuffer[2 * i + 0] = destination[i];
		txBuffer[2 * i + 1] = value[i];
    }*/
	txBuffer[0] = LIS3DH_REG_CTRL1;
	txBuffer[1] = 0x07; 	//all axes, normal mode (0x07)
	txBuffer[2] = LIS3DH_REG_CTRL4;
	txBuffer[3] = 0x88;		 	//high res and BDU enabled
	txBuffer[4] = LIS3DH_REG_CTRL3;
	txBuffer[5] = 0x10;			//DRDY on INT1
	txBuffer[6] = LIS3DH_REG_TEMPCFG;
	txBuffer[7] = 0x80;			//enable adcs
	txBuffer[8] = LIS3DH_REG_OUT_X_L;
	txBuffer[9] = 0x80;			//for auto increment

    //print out tx details

    for(i = 0; i < 10; i++)
    	System_printf("%x ",txBuffer[i]);
    System_printf("\ndestination %i\n",sizeof(destination));
    System_printf("value %i\n",sizeof(value));



    t_i2cTransaction.writeBuf = txBuffer;
    //t_i2cTransaction.writeCount = (sizeof(destination)+sizeof(value) + 2);	//sizeof array gets -1 of actual size
    t_i2cTransaction.writeCount = 64;
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = Board_LIS3DH_ADDR; //0x18

    t_params.bitRate = I2C_400kHz;
	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer in callback mode (doesn't stall system)
	I2C_transfer(t_handle, &t_i2cTransaction);

	while(!transferDone){
		//loop until transfer is complete
		System_printf("Waiting for transferDone\n");
		System_flush();
		Task_sleep(1000000 / Clock_tickPeriod);
	}

	/*Deinitialized I2C */
	I2C_close(t_handle);
	System_printf("I2C closed\n");
	transferDone = false;
	receiveStart = true;
	System_flush();

	return;
}

Void initLIS3DH(UArg arg0, UArg arg1){
	unsigned int	i;
	uint16_t		acceleration;

    //all axes , normal mode
    //high res and BDU enabled
    //DRDY on INT1
    //enable adcs
    //enable auto increment
    uint8_t			LIS3DH_Init_Dest_Reg[5] = {LIS3DH_REG_CTRL1, LIS3DH_REG_CTRL4, LIS3DH_REG_CTRL3, LIS3DH_REG_TEMPCFG, LIS3DH_REG_OUT_X_L};
    uint8_t			LIS3DH_Init_Values[5] =  {0x07, 0x88, 0x10, 0x80, 0x80};
    uint8_t         txBuffer[sizeof(LIS3DH_Init_Dest_Reg)+sizeof(LIS3DH_Init_Values) + 2];
    uint8_t         rxBuffer[2];

    I2C_Transaction i2cTransaction;

    //locals
    I2C_Handle handle;
    I2C_Params params;


    //writeI2CRegister(LIS3DH_Init_Dest_Reg, LIS3DH_Init_Values);

    // Configure I2C parameters.
    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_CALLBACK;
    params.transferCallbackFxn = transferCallback;

    /*prepare data to send*/
//    txBuffer[0] = LIS3DH_REG_CTRL1;
//    txBuffer[1] = 0x07; 	//all axes, normal mode (0x07)
//    txBuffer[2] = LIS3DH_REG_CTRL4;
//    txBuffer[3] = 0x88;		 	//high res and BDU enabled
//    txBuffer[4] = LIS3DH_REG_CTRL3;
//    txBuffer[5] = 0x10;			//DRDY on INT1
//    txBuffer[6] = LIS3DH_REG_TEMPCFG;
//    txBuffer[7] = 0x80;			//enable adcs
//    txBuffer[8] = LIS3DH_REG_OUT_X_L;
//    txBuffer[9] = 0x80;			//for auto increment
    for(i = 0; i < sizeof(LIS3DH_Init_Dest_Reg) + 1; i++){
        	txBuffer[2 * i + 0] = LIS3DH_Init_Dest_Reg[i];
    		txBuffer[2 * i + 1] = LIS3DH_Init_Values[i];
    }
    		/*initialize master I2C transaction structure*/

    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = (sizeof(LIS3DH_Init_Dest_Reg)+sizeof(LIS3DH_Init_Values) + 2)*2;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    i2cTransaction.slaveAddress = Board_LIS3DH_ADDR; //0x18

    /* Create I2C for usage */
    //I2C_Params_init(&i2cParams);
    params.bitRate = I2C_400kHz;
    handle = I2C_open(Board_I2C, &params);
    if (handle == NULL) {
        System_abort("Error Initializing I2C for Transmitting\n");
    }
    else {
        System_printf("I2C Initialized for Transmitting!\n");
    }

    //do i2c transfer in callback mode (doesn't stall system)
    I2C_transfer(handle, &i2cTransaction);

    while(!transferDone){
    	//loop until transfer is complete
    	System_printf("Waiting for transferDone\n");
    	System_flush();
    	Task_sleep(1000000 / Clock_tickPeriod);
    }

    /*Deinitialized I2C */
    I2C_close(handle);
    System_printf("I2C closed. transfer finished\n");

    System_flush();

    /*
    while(!receiveStart){
    	System_printf("Waiting for transmit to finish before receiving\n");
		System_flush();
		Task_sleep(1000000 / Clock_tickPeriod);
    } */
    /*receiving now*/

    /*configure I2C parameters*/
    I2C_Params_init(&params);

    /*initialize master I2C transaction structure*/
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 0;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 16;
    i2cTransaction.slaveAddress = Board_LIS3DH_ADDR; //0x18

    /*open i2c*/
    handle = I2C_open(Board_I2C, &params);
    if (handle == NULL) {
        System_abort("Error Initializing I2C for Receiving\n");
    }
    else {
        System_printf("I2C Initialized for Receiving!\n");
    }

	System_flush();

    /*take a few samples and print into console*/
    for(i = 1; i< 100; i++){
    	if (I2C_transfer(handle, &i2cTransaction)){
    		acceleration = (float)(uint16_t)((rxBuffer[i] << 8 | rxBuffer[i-1]) / 16380); //16380 is the divider for 2G range
    		if(i%3 == 1)
    			System_printf("x%u: %d (g)      ", i, acceleration);
    		if(i%3 == 2)
    			System_printf("y%u: %d (g)      ", i, acceleration);
    		if(i%3 == 0){
    			System_printf("z%u: %d (g)\n", i, acceleration);
    	    	//Task_sleep(1000000 / Clock_tickPeriod);
    	    	System_flush();
    		}
    	}
    	else{
    		System_printf("I2C Bus fault");
    		//System_printf("      (rx buffer: %d)\n",acceleration );
    	}
    }

    /*Deinitialized I2C */
    I2C_close(handle);
    System_printf("I2C closed receiving finished\n");

    System_flush();
    receiveStart = false;
}


/*
 *  ======== main ========
 */
int main(void)
{
    PIN_Handle ledPinHandle;
    Task_Params taskParams;

    /* Call board init functions */
    Board_initGeneral();
    //Board_initUART();
    Board_initI2C();

    /* Construct BIOS objects */
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)initLIS3DH, &taskParams, NULL);

    /* Open LED pins */
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if(!ledPinHandle) {
        System_abort("Error initializing board LED pins\n");
    }

    PIN_setOutputValue(ledPinHandle, Board_LED1, 1);

    /* This example has logging and many other debug capabilities enabled */
    System_printf("This example does not attempt to minimize code or data "
                  "footprint\n");
    System_flush();

    System_printf("Starting I2C Test\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in "
                  "ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
