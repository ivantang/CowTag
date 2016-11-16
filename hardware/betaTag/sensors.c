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

#include <sensors.h>

/*boolean to show if transfer is done*/
bool	transferDone = false;
/*boolean to show when ready to receive*/
bool	receiveStart = false;
bool	verbose = false;	//print out debug messages when true

/**constants*/
#define TASKSTACKSIZE		1024	//i2c

/**structures*/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];


/*function definition */
void Sensors_init(void){
    Task_Params taskParams;

	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)testSensors, &taskParams, NULL);
}

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

//callback function for i2c callback mode, currently not using blocking mode so not using this
//can implement this later when needed
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

//sends 8bit value to target i2c board address
static void writeI2C(uint8_t board_address, uint8_t value){
	unsigned int 	i;
	uint8_t			txBuffer[1];
	uint8_t         rxBuffer[1];

	I2C_Transaction t_i2cTransaction;
	I2C_Handle 		t_handle;
	I2C_Params		t_params;

	I2C_Params_init(&t_params);
    t_params.transferMode = I2C_MODE_BLOCKING;
    t_params.bitRate = I2C_400kHz;

    /*prepare data to send*/
    txBuffer[0] = value;

    t_i2cTransaction.writeBuf = txBuffer;
    t_i2cTransaction.writeCount = 1;
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = board_address; //0x18


	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		//if(verbose) System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer
	I2C_transfer(t_handle, &t_i2cTransaction);

	/*Deinitialized I2C */
	I2C_close(t_handle);
	//if(verbose)	System_printf("write closed\n");
	System_flush();
}

//sends 8bit value to target address on target board address
//first 8 bits in txbuffer is address on hardware we want to write to
//seconds 8 bits in txbuffer is value we want to write
static void writeI2CRegister(uint8_t board_address, uint8_t destination, uint8_t value){
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
    t_i2cTransaction.writeCount = 2;
    t_i2cTransaction.readBuf = rxBuffer;
    t_i2cTransaction.readCount = 0;
    t_i2cTransaction.slaveAddress = board_address; //0x18


	t_handle = I2C_open(Board_I2C, &t_params);
	if (t_handle == NULL) {
		System_abort("Error Initializing I2C for Transmitting\n");
	}
	else {
		//if(verbose) System_printf("I2C Initialized for Transmitting!\n");
	}

	//do i2c transfer
	I2C_transfer(t_handle, &t_i2cTransaction);

	/*Deinitialized I2C */
	I2C_close(t_handle);
	//if(verbose)	System_printf("write closed\n");
	System_flush();
}

//similar to writeI2CRegister but instead takes arrays as arguments
static void writeI2CRegisters(int8_t board_address, uint8_t destination[], uint8_t value[]){
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

//reads 8bit * 3 from target address
static uint32_t readI2CWord(uint8_t board_address, uint8_t address){
	uint8_t			txBuffer[1] = {address};
	uint8_t			rxBuffer[3];

	I2C_Transaction i2cTransaction;
	I2C_Handle handle;
	I2C_Params params;

    I2C_Params_init(&params);
    params.transferMode = I2C_MODE_BLOCKING;
    params.bitRate = I2C_400kHz;

    i2cTransaction.writeBuf = txBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = rxBuffer;
	i2cTransaction.readCount = 3;
	i2cTransaction.slaveAddress = board_address;

	handle = I2C_open(Board_I2C, &params);
	if (handle == NULL) {
		System_abort("Error Initializing I2C\n");
	}
	else {
		//if(verbose)System_printf("I2C Initialized!\n");
	}
	System_flush();

    I2C_transfer(handle, &i2cTransaction);

    if(verbose)System_printf("rxBuffer: 0x%x%x%x read from 0x%x\n",rxBuffer[1],rxBuffer[0],rxBuffer[2],address);
    System_flush();

    I2C_close(handle);
    return (rxBuffer[2] | rxBuffer[0] << 8 | rxBuffer[1] << 16);
	//if(verbose)	System_printf("read closed\n");
	System_flush();
}

//input board address and address of register you want to read
//returns 8bit value in the register
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
		//if(verbose)System_printf("I2C Initialized!\n");
	}
	System_flush();

    I2C_transfer(handle, &i2cTransaction);

    if(verbose)System_printf("rxBuffer: 0x%x read from 0x%x\n",rxBuffer[0],address);
    System_flush();

    I2C_close(handle);
    return rxBuffer[0];
	//if(verbose)	System_printf("read closed\n");
	System_flush();
}

Void getAcceleration(){
	if(verbose)System_printf("whoamI: 0x%x \n", readI2CRegister(Board_LIS3DH_ADDR, 15)); //should read 0x33
	System_flush();

	unsigned int	i;
	uint16_t		x, y, z;

    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL1, 0x77);    //all axes , normal mode
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL4, 0x0A);	//high res and BDU and self test 1
    //writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL3, 0x10);    //DRDY on INT1
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_TEMPCFG, 0x80);    //enable adcs
    //writeI2C(Board_LIS3DH_ADDR, LIS3DH_REG_OUT_X_L | 0x80);    //enable auto increment

    //polling status register to check for new set of data
    for(i = 0 ; i < 30 ; i++){
    	if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) & 0x8) >> 3 == 1 ){
    		if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) >> 7) == 1 ){
    			x = readI2CRegister(Board_LIS3DH_ADDR,0x28) | (readI2CRegister(Board_LIS3DH_ADDR,0x29) << 8) ;
    			y = readI2CRegister(Board_LIS3DH_ADDR,0x2A) | (readI2CRegister(Board_LIS3DH_ADDR,0x2B) << 8) ;
    			z = readI2CRegister(Board_LIS3DH_ADDR,0x2C) | (readI2CRegister(Board_LIS3DH_ADDR,0x2D) << 8) ;
    			System_printf("x:%d y:%d z:%d\n", x ,y, z);
    			System_flush();
    		}
    	}
    }

    if(verbose)System_printf("\nI2C closed receiving finished\n");
    System_flush();
}

Void getObjTemp(){
	uint32_t temp_l, temp_h, flags;
	uint8_t	 pec;
	int i;

	//System_printf("i am 0x%x\n", readI2CRegister(Board_MIKROE1362_ADDR,0x0E));
	//System_flush();


	flags = readI2CRegister(Board_MIKROE1362_ADDR << 1,0xF0);
	System_printf("flags:0x%x \n",flags);

	for(i = 0 ; i < 5 ; i++){
		//writeI2C(Board_MIKROE1362_ADDR,0xB4);
		temp_l = readI2CWord(Board_MIKROE1362_ADDR << 1,0x07);
		//temp_h = readI2CRegister(Board_MIKROE1362_ADDR << 1,0x07);
		//pec = readI2CRegister(Board_MIKROE1362_ADDR << 1,0x07);
		System_printf("temp:0x%x\n", temp_l);
		//System_printf("temp:0x%x 0x%x 0x%x\n",temp_h,temp_l,pec);
		System_flush();
	}
}


Void initMIKROE1362_1(){
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

Void getHeartRate(){
	unsigned int	i;
	uint8_t previous;
	uint8_t	heartRate;
	//set mode
	writeI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_MODE_CONFIGURATION , MAX30100_MODE_HRONLY);

	//set leds pulse width
	previous = readI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION);
	writeI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION , ((previous & 0xfc) | MAX30100_SPC_PW_1600US_16BITS) );

	//set sampling rate
	previous = readI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION);
	writeI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION , (previous & 0xe3) | (MAX30100_SAMPRATE_100HZ << 2));

	//set led current
	writeI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_LED_CONFIGURATION , MAX30100_LED_CURR_50MA << 4 | MAX30100_LED_CURR_50MA );

	//set high res mode enable
	previous = readI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION);
	writeI2CRegister( Board_MAX30100_ADDR , MAX30100_REG_SPO2_CONFIGURATION , previous | MAX30100_SPC_SPO2_HI_RES_EN );

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

void testSensors(){
	getAcceleration();
	getObjTemp();
	getHeartRate();
}
