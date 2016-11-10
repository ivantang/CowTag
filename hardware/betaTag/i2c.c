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
bool	verbose = true;	//print out debug messages when true
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
        if(verbose) System_printf("Transaction complete\n");
    } else {
        // Transaction failed, act accordingly...
        System_abort("Transaction failed\n");
    }
    System_flush();
}

static void writeI2CValue(uint8_t board_address, uint8_t destination, uint8_t value){
	unsigned int 	i;
	uint8_t			txBuffer[2];
	uint8_t         rxBuffer[1];

	I2C_Transaction t_i2cTransaction;
	I2C_Handle 		t_handle;
	I2C_Params		t_params;

	I2C_Params_init(&t_params);
    t_params.transferMode = I2C_MODE_BLOCKING;
    t_params.bitRate = I2C_400kHz;

    /*prepare data to send*/
    txBuffer[0] = destination;
    txBuffer[1] = value;

    t_i2cTransaction.writeBuf = txBuffer;
    t_i2cTransaction.writeCount = 16;	//sizeof array gets -1 of actual size
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = board_address; //0x18


	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		if(verbose) System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer
	I2C_transfer(t_handle, &t_i2cTransaction);

	/*Deinitialized I2C */
	I2C_close(t_handle);
	if(verbose)	System_printf("write closed\n");
	System_flush();
}

static void writeI2CRegister(uint8_t board_address, uint8_t destination[], uint8_t value[]){
	unsigned int 	i;
	uint8_t			txBuffer[sizeof(destination)+sizeof(value) + 2];
	uint8_t         rxBuffer[1];

	I2C_Transaction t_i2cTransaction;
	I2C_Handle 		t_handle;
	I2C_Params		t_params;

	I2C_Params_init(&t_params);
    t_params.transferMode = I2C_MODE_BLOCKING;
    t_params.bitRate = I2C_400kHz;

    /*check if destination and value arrays are same size*/
    assert(sizeof(destination) == sizeof(value));

    /*prepare data to send*/
    for(i = 0; i < sizeof(destination) + 1; i++){
    	txBuffer[2 * i + 0] = destination[i];
		txBuffer[2 * i + 1] = value[i];
    }

//print out tx details

//    for(i = 0; i < 10; i++)
//    	System_printf("%x ",txBuffer[i]);
//    System_printf("\ndestination %i\n",sizeof(destination));
//    System_printf("value %i\n",sizeof(value));

    t_i2cTransaction.writeBuf = txBuffer;
    t_i2cTransaction.writeCount = (sizeof(txBuffer) + 1)*2;	//sizeof array gets -1 of actual size
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = board_address; //0x18


	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		if(verbose) System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer
	I2C_transfer(t_handle, &t_i2cTransaction);

	/*Deinitialized I2C */
	I2C_close(t_handle);
	if(verbose)	System_printf("write closed\n");
	System_flush();
}

//input board address and address of register you want to read
//returns value in the register
static uint8_t readI2CRegister(uint8_t board_address, uint8_t address){
	uint8_t			txBuffer[1] = {address};
	uint8_t			rxBuffer[1];

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_400kHz;

    i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 1;
	i2cTransaction.slaveAddress = board_address;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}
	else {
		if(verbose)System_printf("I2C Initialized!\n");
	}
	System_flush();

    I2C_transfer(handle, &i2cTransaction);

    if(verbose)System_printf("rxBuffer: %x\n",rxBuffer[0]);
    System_flush();

    I2C_close(handle);
    return rxBuffer[0];
	if(verbose)	System_printf("read closed\n");
	System_flush();
}

Void initLIS3DH(){
	unsigned int	i;
	uint16_t		acceleration;
	//float			acceleration;	//TODO fix this

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

    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_Init_Dest_Reg, LIS3DH_Init_Values);		//currently not using this function

    // Configure I2C parameters.
    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_400kHz;
    //params.transferCallbackFxn = transferCallback;

//    //making txbuffer array
//    for(i = 0; i < sizeof(LIS3DH_Init_Dest_Reg) + 1; i++){
//        	txBuffer[2 * i + 0] = LIS3DH_Init_Dest_Reg[i];
//    		txBuffer[2 * i + 1] = LIS3DH_Init_Values[i];
//    }
//
//    /*initialize master I2C transaction structure*/
//    i2cTransaction.writeBuf = txBuffer;
//    i2cTransaction.writeCount = (sizeof(LIS3DH_Init_Dest_Reg)+sizeof(LIS3DH_Init_Values) + 2)*2;
//    i2cTransaction.readBuf = rxBuffer;
//    i2cTransaction.readCount = 0;
//    i2cTransaction.slaveAddress = Board_LIS3DH_ADDR; //0x18
//
//    /* Create I2C for usage */
//    params.bitRate = I2C_400kHz;
//    handle = I2C_open(Board_I2C, &params);
//    if (handle == NULL) {
//        System_abort("Error Initializing I2C for Transmitting\n");
//    }
//    else {
//    	if(verbose)System_printf("I2C Initialized for Transmitting!\n");
//    }
//
//    //do i2c transfer in callback mode (doesn't stall system)
//    I2C_transfer(handle, &i2cTransaction);
//
//    while(!transferDone){
//    	//loop until transfer is complete
//    	if(verbose)	System_printf("Waiting for transferDone\n");
//    	System_flush();
//    	Task_sleep(1000000 / Clock_tickPeriod);
//    }
//
//    /*Deinitialized I2C */
//    I2C_close(handle);
//    if(verbose)System_printf("I2C closed. transfer finished\n");
//    System_flush();

    /*
    while(!receiveStart){
    	System_printf("Waiting for transmit to finish before receiving\n");
		System_flush();
		Task_sleep(1000000 / Clock_tickPeriod);
    } */

    /*RECEIVING NOW*/

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
    	if(verbose)System_printf("I2C Initialized for Receiving!\n");
    }

	System_flush();

    /*take a few samples and print into console*/
    for(i = 1; i< 32; i++){
    	if (I2C_transfer(handle, &i2cTransaction)){
    		acceleration = (float)(uint16_t)((rxBuffer[i] << 8 | rxBuffer[i-1]) / 16380); //16380 is the divider for 2G range
    		if(i % 3 == 1)
    			System_printf("x%u: %4.2d (g)      ", i, acceleration);
    		if(i % 3 == 2)
    			System_printf("y%u: %4.2d (g)      ", i, acceleration);
    		if(i % 3 == 0){
    			System_printf("z%u: %4.2d (g)\n", i, acceleration);
    	    	//Task_sleep(1000000 / Clock_tickPeriod);
    	    	System_flush();
    		}
    	}
    	else{
    		System_abort("I2C Bus fault");
    		//System_printf("      (rx buffer: %d)\n",acceleration );
    	}
    }

    /*Deinitialized I2C */
    I2C_close(handle);
    if(verbose)System_printf("I2C closed receiving finished\n");

    System_flush();
}

Void initLIS3DH1(){
	unsigned int	i;
	uint16_t		acceleration;
	//float			acceleration;	//TODO fix this

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

    //writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_Init_Dest_Reg, LIS3DH_Init_Values);		//currently not using this function

    // Configure I2C parameters.
    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_CALLBACK;
    params.transferCallbackFxn = transferCallback;

    //making txbuffer array
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
    params.bitRate = I2C_400kHz;
    handle = I2C_open(Board_I2C, &params);
    if (handle == NULL) {
        System_abort("Error Initializing I2C for Transmitting\n");
    }
    else {
    	if(verbose)System_printf("I2C Initialized for Transmitting!\n");
    }

    //do i2c transfer in callback mode (doesn't stall system)
    I2C_transfer(handle, &i2cTransaction);

    while(!transferDone){
    	//loop until transfer is complete
    	if(verbose)	System_printf("Waiting for transferDone\n");
    	System_flush();
    	Task_sleep(1000000 / Clock_tickPeriod);
    }

    /*Deinitialized I2C */
    I2C_close(handle);
    if(verbose)System_printf("I2C closed. transfer finished\n");
    System_flush();

    /*
    while(!receiveStart){
    	System_printf("Waiting for transmit to finish before receiving\n");
		System_flush();
		Task_sleep(1000000 / Clock_tickPeriod);
    } */

    /*RECEIVING NOW*/

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
    	if(verbose)System_printf("I2C Initialized for Receiving!\n");
    }

	System_flush();

    /*take a few samples and print into console*/
    for(i = 1; i< 32; i++){
    	if (I2C_transfer(handle, &i2cTransaction)){
    		acceleration = (float)(uint16_t)((rxBuffer[i] << 8 | rxBuffer[i-1]) / 16380); //16380 is the divider for 2G range
    		if(i % 3 == 1)
    			System_printf("x%u: %4d (g)      ", i, acceleration);
    		if(i % 3 == 2)
    			System_printf("y%u: %4d (g)      ", i, acceleration);
    		if(i % 3 == 0){
    			System_printf("z%u: %4d (g)\n", i, acceleration);
    	    	//Task_sleep(1000000 / Clock_tickPeriod);
    	    	System_flush();
    		}
    	}
    	else{
    		System_abort("I2C Bus fault");
    		//System_printf("      (rx buffer: %d)\n",acceleration );
    	}
    }

    /*Deinitialized I2C */
    I2C_close(handle);
    if(verbose)System_printf("I2C closed receiving finished\n");

    System_flush();
    receiveStart = false;
}

Void initMIKROE1362(){
	unsigned int	i;
	uint16_t		temperature;
	float			celcius;
	//uint8_t			MIKROE1362_Init_Dest_Reg[5] = {( Board_MIKROE1362_ADDR )};
	//uint8_t			MIKROE1362_Init_Values[5] =  { MIKROE1362_AMB_TEMP };
	//uint8_t         txBuffer[sizeof(MIKROE1362_Init_Dest_Reg)+sizeof(MIKROE1362_Init_Values) + 2];
	uint8_t			txBuffer[32];
	uint8_t         rxBuffer[32];

	I2C_Transaction i2cTransaction;
	I2C_Transaction	i2cTransaction2;

	//locals
	I2C_Handle handle;
	I2C_Params params;

	//configure i2c params
    I2C_Params_init(&params);
    //params.transferMode = I2C_MODE_CALLBACK;
    //params.transferCallbackFxn = transferCallback;

	/*prepare data to send*/
    txBuffer[0] = MLX90614_TOBJ1; //object temperature

    /*initialize master I2C transaction structure*/
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 4;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 0;
    i2cTransaction.slaveAddress = Board_MIKROE1362_ADDR;

    /* Create I2C for usage */
    params.bitRate = I2C_400kHz;
    handle = I2C_open(Board_I2C, &params);
    if (handle == NULL) {
        System_abort("Error Initializing MIKROE1362 for Transmitting\n");
    }
    else {
    	if(verbose)System_printf("MIKROE1362 Initialized for Transmitting!\n");
    }

    //do i2c transfer in blocking mode (stall system)
    I2C_transfer(handle, &i2cTransaction);


    /*Deinitialized I2C */
    I2C_close(handle);
    if(verbose) System_printf("Transfer finished\n");
    System_flush();



    /*RECEIVING NOW*/
    I2C_Params_init(&params);

    i2cTransaction2.writeBuf = txBuffer;
    i2cTransaction2.writeCount = 0;
    i2cTransaction2.readBuf = rxBuffer;
    i2cTransaction2.readCount = 12;
    i2cTransaction2.slaveAddress = Board_MIKROE1362_ADDR;

    handle = I2C_open(Board_I2C, &params);
    if (handle == NULL) {
        System_abort("Error Initializing I2C for Receiving\n");
    }
    else {
    	if(verbose) System_printf("I2C Initialized for Receiving!\n");
    }
	System_flush();

    /*take a few samples and print into console*/
    for(i = 1; i< 17; i++){
    	if (I2C_transfer(handle, &i2cTransaction2)){
    		temperature = rxBuffer[i];
    		temperature |= (rxBuffer[i] << 8);
    		celcius = temperature;
    		celcius *= .02;
    		celcius -= 273.15;
    		celcius = 1 ;
   	    	System_printf("temperature%u:", i);
   	    	System_printf("%5.2f ",celcius);
	    	System_flush();
   	    	Task_sleep(100000 / Clock_tickPeriod);
   	    	if( i % 4 == 0 ){
   	    		System_printf("\n");

   	    	}
    	}
    	else{
    		System_abort("I2C Bus fault\n");
    		break;
    	}
    }

    /*Deinitialized I2C */
    I2C_close(handle);
    if(verbose) System_printf("\nI2C closed receiving finished\n");

    System_flush();
    receiveStart = false;
}

Void initMAX30100(){
	unsigned int	i;
	uint8_t previous;
	uint8_t	heartRate;
	//set mode
	writeI2CValue( Board_MAX30100_ADDR , MAX30100_REG_MODE_CONFIGURATION , MAX30100_MODE_HRONLY);

	//set leds pulse width
	previous = readI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION);
	writeI2CValue( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION , ((previous & 0xfc) | MAX30100_SPC_PW_1600US_16BITS) );

	//set sampling rate
	previous = readI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION);
	writeI2CValue( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION , (previous & 0xe3) | (MAX30100_SAMPRATE_100HZ << 2));

	//set led current
	writeI2CValue( Board_MAX30100_ADDR , MAX30100_REG_LED_CONFIGURATION , MAX30100_LED_CURR_50MA << 4 | MAX30100_LED_CURR_50MA );

	//set high res mode enable
	previous = readI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION);
	writeI2CValue( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION , previous | MAX30100_SPC_SPO2_HI_RES_EN );

	uint8_t         rxBuffer[4];
	I2C_Transaction i2cTransaction;

	//locals
	I2C_Handle handle;
	I2C_Params params;
    I2C_Params_init(&params);

	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 20;
	i2cTransaction.slaveAddress = Board_MAX30100_ADDR; //0x18

	handle = I2C_open(Board_I2C, &params);
		if (handle == NULL) {
			System_abort("Error Initializing I2C for Receiving\n");
		}
		else {
			System_printf("I2C Initialized for Receiving!\n");
		}

		System_flush();

		/*take a few samples and print into console*/
		for(i = 0 ; i < 20 ; i++){
			heartRate = rxBuffer[0];
			System_printf("Heart rate: %2d    ",heartRate);
			if(i % 5 == 0){
				System_printf("\n");
			}
		}

		/*Deinitialized I2C */
		I2C_close(handle);
		System_printf("I2C closed receiving finished\n");
		System_flush();


}
