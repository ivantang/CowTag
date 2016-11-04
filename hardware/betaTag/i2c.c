/*
 * i2c.c
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

/* TI-RTOS Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/I2C.h> //i2c
#include <ti/drivers/UART.h> //i2c

/* Example/Board Header files */
#include "Board.h"

#include <stdint.h>
#include <assert.h>

#include "i2c.h"

/*boolean to show if transfer is done*/
bool	transferDone = false;
/*boolean to show when ready to receive*/
bool	receiveStart = false;

/**************************************************************************/

/*
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




