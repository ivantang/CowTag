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
#include <debug.h>
#include <stdint.h>
#include <assert.h>

#include <sensors.h>
#include <IIC.c>
#include <IIC.h>



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

//sends data to uart output. can be read via com
void echoFxn(UArg arg0, UArg arg1)
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

struct accelerationData getAcceleration(){
	if(verbose_sensors)System_printf("\n\nwhoamI: 0x%x \n", readI2CRegister(Board_LIS3DH_ADDR, 15)); //should read 0x33
	System_flush();

	unsigned int	i;
	struct accelerationData accelerationdata;

    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL1, 0x77);    //all axes , normal mode
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL4, 0x0A);	//high res and BDU and self test 1
    //writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL3, 0x10);    //DRDY on INT1
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_TEMPCFG, 0x80);    //enable adcs
    //writeI2C(Board_LIS3DH_ADDR, LIS3DH_REG_OUT_X_L | 0x80);    //enable auto increment

    //polling status register to check for new set of data
    for(i = 0 ; i < 30 ; i++){
    	if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) & 0x8) >> 3 == 1 ){
    		if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) >> 7) == 1 ){
    			accelerationdata.x = readI2CRegister(Board_LIS3DH_ADDR,0x28) | (readI2CRegister(Board_LIS3DH_ADDR,0x29) << 8) ;
    			accelerationdata.y = readI2CRegister(Board_LIS3DH_ADDR,0x2A) | (readI2CRegister(Board_LIS3DH_ADDR,0x2B) << 8) ;
    			accelerationdata.z = readI2CRegister(Board_LIS3DH_ADDR,0x2C) | (readI2CRegister(Board_LIS3DH_ADDR,0x2D) << 8) ;
    			if(verbose_sensors) System_printf("x:%d y:%d z:%d\n", accelerationdata.x ,accelerationdata.y, accelerationdata.z);
    			System_flush();
    			break;
    		}
    	}
    }

    System_flush();

    return accelerationdata;
}

struct temperatureData getObjTemp(){
	struct temperatureData temperaturedata;
	uint32_t flags;
	uint8_t	 pec;

	int i;

	//if(verbose_sensors)System_printf("\n\ni am 0x%x\n", readI2CRegister100kHz(Board_MIKROE1362_ADDR,0x2E));
//	if(verbose_sensors)System_printf("flag 0x%x\n", readI2CWord100kHz(Board_MIKROE1362_ADDR,0xF0));
//	if(verbose_sensors)System_printf("id 1 0x%x\n", readI2CWord100kHz(Board_MIKROE1362_ADDR,0x3C));
//	if(verbose_sensors)System_printf("id 2 0x%x\n", readI2CWord100kHz(Board_MIKROE1362_ADDR,0x3D));
//	if(verbose_sensors)System_printf("id 3 0x%x\n", readI2CWord100kHz(Board_MIKROE1362_ADDR,0x3E));
//	if(verbose_sensors)System_printf("id 4 0x%x\n", readI2CWord100kHz(Board_MIKROE1362_ADDR,0x3F));
//	if(verbose_sensors)System_printf("config register 0x%x\n", readI2CWord100kHz(Board_MIKROE1362_ADDR,0x25));
	//writeI2C(Board_MIKROE1362_ADDR, 0xFF);    //all axes , normal mode


	//if(verbose_sensors) System_printf("temp_obj 0x%x\n",readI2CWord100kHz(Board_MIKROE1362_ADDR,0x06) >> 8);
	//if(verbose_sensors) System_printf("temp_obj 0x%x\n",readI2CWord100kHz(Board_MIKROE1362_ADDR,0x06));
	temperaturedata.temp_h = readI2CWord100kHz(Board_MIKROE1362_ADDR,0x06) >> 8;


//	flags = readI2CRegister(Board_MIKROE1362_ADDR << 1,0xF0);
//	System_printf("flags:0x%x \n",flags);
//
//	for(i = 0 ; i < 5 ; i++){
//		//writeI2C(Board_MIKROE1362_ADDR,0xB4);
//		temperaturedata.temp_l = readI2CWord(Board_MIKROE1362_ADDR << 1,0x07);
//		temperaturedata.temp_h = readI2CRegister(Board_MIKROE1362_ADDR << 1,0x07);
//		//pec = readI2CRegister(Board_MIKROE1362_ADDR << 1,0x07);
//		System_printf("temp:0x%x\n", temp_l);
//		//System_printf("temp:0x%x 0x%x 0x%x\n",temp_h,temp_l,pec);
//		System_flush();
//	}
	//System_flush();
	return temperaturedata;
}


struct heartrateData getHeartRate(){
	struct heartrateData heartratedata;
	int write_reg, read_reg;
	int i;
	//check if device is connected
	if(readI2CRegister(Board_MAX30100_ADDR,0xFF) != 0x11){
		System_printf("Hardware is not the MAX30100 : 0x%x\n",readI2CRegister(Board_MAX30100_ADDR,0xFF));
		System_flush();
	}
	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_MODE_CONFIGURATION, 0x02);	//enable HR only

	//writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_LED_CONFIGURATION, 0xFF);

	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_MODE_CONFIGURATION, 0x0A);	//get temp reading
	heartratedata.rate_h = (unsigned int)readI2CRegister(Board_MAX30100_ADDR, 0x16);
	heartratedata.rate_l = (unsigned int)readI2CRegister(Board_MAX30100_ADDR, 0x17);
	if(verbose_sensors) System_printf("temperature: %x.%x\n", heartratedata.rate_h, heartratedata.rate_l);
	System_flush();

	writeI2CRegister(Board_MAX30100_ADDR, 0x01, 0xE0);	//turn on interrupts
	while((readI2CRegister(Board_MAX30100_ADDR, 0x00) & 0x20) != 0x20){	//check if hr data is ready
		//get read pointer
		//readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_READ_POINTER);

		//currently writing to this register
		if(verbose_sensors) System_printf("FIFO Write Pointer:0x%x\n", readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_WRITE_POINTER));
		//if reading fifo data, data is from this register
		if(verbose_sensors) System_printf("FIFO Read Pointer:0x%x\n", readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_READ_POINTER));

		//write_reg = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_WRITE_POINTER);
		//read_reg = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_READ_POINTER);

		//System_printf("Avaliable samples:0x%x\n", write_reg-read_reg);
		//fifo data is 16 bits so 4 reads is needed
		//first 16 bits is IR data, in our case, HR data
		heartratedata.rate_h = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
		if(verbose_sensors) System_printf("IR_Data_H:0x%x ", heartratedata.rate_h);
		heartratedata.rate_l = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
		if(verbose_sensors) System_printf("IR_Data_L:0x%x\n", heartratedata.rate_l);


		//RED LED data, we dont need this unless looking at Oximeter
		//System_printf("FIFO Data:0x%x\n", readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA));
		//System_printf("FIFO Data:0x%x\n", readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA));
		System_flush();
		//break;
	}
	//print out interrupt status register
	if(verbose_sensors) System_printf("Interrupt Reg: 0x%x\n", readI2CRegister(Board_MAX30100_ADDR, 0x00));
	System_flush();

	return heartratedata;
}


void testSensors(){
	getAcceleration();
	//getObjTemp();
	getHeartRate();
	System_printf("Tests done\n");
	System_flush();
}
