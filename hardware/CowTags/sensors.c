/*
 * i2c.c
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */


/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>

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
#include <IIC.h>

PIN_Config BoardGpioInitialTable[] = {
//		Board_I2C0_SDA0 | PIN_GPIO_LOW,
//		Board_I2C0_SCL0	| PIN_GPIO_HIGH,
		PIN_TERMINATE
};

PIN_State pinState;

/**constants*/
#define TASKSTACKSIZE		1024	//i2c

/**structures*/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*function definition */
void Sensors_init(void){
	//PIN_init(BoardGpioInitialTable);
	Task_Params taskParams;

	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)testSensors, &taskParams, NULL);
}


void getAcceleration(struct sampleData *sampleData){
	//if(verbose_sensors)System_printf("\n\nwhoamI: 0x%x \n", readI2CRegister(Board_LIS3DH_ADDR, 15)); //should read 0x33
	//System_flush();

	unsigned int	i;

    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL1, 0x77);    //all axes , normal mode
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL4, 0x08);	//high res and BDU and self test off
    //writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL3, 0x10);    //DRDY on INT1
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_TEMPCFG, 0x80);    //enable adcs
    //writeI2C(Board_LIS3DH_ADDR, LIS3DH_REG_OUT_X_L | 0x80);    //enable auto increment

    //polling status register to check for new set of data
    for(i = 0 ; i < 30 ; i++){
    	if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) & 0x8) >> 3 == 1 ){
    		if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) >> 7) == 1 ){
    			sampleData->accelerometerData.x = readI2CRegister(Board_LIS3DH_ADDR,0x28) | (readI2CRegister(Board_LIS3DH_ADDR,0x29) << 8);
    			sampleData->accelerometerData.y = readI2CRegister(Board_LIS3DH_ADDR,0x2A) | (readI2CRegister(Board_LIS3DH_ADDR,0x2B) << 8) ;
    			sampleData->accelerometerData.z = readI2CRegister(Board_LIS3DH_ADDR,0x2C) | (readI2CRegister(Board_LIS3DH_ADDR,0x2D) << 8) ;

    			if(verbose_sensors) System_printf("x:%d y:%d z:%d\n", 	sampleData->accelerometerData.x ,
    																	sampleData->accelerometerData.y,
																		sampleData->accelerometerData.z);
    			System_flush();
    			break;
    		}
    	}
    }

    System_flush();

    return;
}

void getTemp(struct sampleData *sampleData){
	uint16_t temp_obj;
	uint16_t temp_amb;

	System_printf("B\n");
	System_flush();

	temp_obj =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x07)*0.02 - 273.15;
	sampleData->tempData.temp_l = temp_obj & 0xFF;
	sampleData->tempData.temp_h = temp_obj >> 8;

	System_printf("B\n");
	System_flush();

	temp_amb =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x06)*0.02 - 273.15;
	sampleData->heartRateData.temp_l = temp_amb & 0xFF;
	sampleData->heartRateData.temp_h = temp_amb >> 8;

	if(verbose_sensors){
		System_printf("temp_obj %d\n", temp_obj);
	}

	return;
}

void getTempNoPtr(){
	uint16_t temp_obj;
	uint16_t temp_amb;

	System_printf("B\n");
	System_flush();

	temp_obj =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x07)*0.02 - 273.15;
	//sampleData->tempData.temp_l = temp_obj & 0xFF;
	//sampleData->tempData.temp_h = temp_obj >> 8;

	System_printf("B\n");
	System_flush();

	temp_amb =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x06)*0.02 - 273.15;
	//sampleData->heartRateData.temp_l = temp_amb & 0xFF;
	//sampleData->heartRateData.temp_h = temp_amb >> 8;

	if(verbose_sensors){
		System_printf("temp_obj %d\n", temp_obj);
		System_printf("temp_obj %d\n", temp_amb);
		System_flush();
	}

	return;
}

void getHeartRate(struct sampleData *sampleData){
	int i = 0;
	uint16_t numValues = 10;
	uint16_t HRData[10];
	uint32_t start_timestamp;
	uint32_t end_timestamp;
	uint32_t start_clock;
	uint32_t end_clock;


	Types_FreqHz frequency;
	//Types_FreqHz *frequency_p = &frequency;

	//check if device is connected
//	if(readI2CRegister(Board_MAX30100_ADDR,0xFF) != 0x11){
//		System_printf("Hardware is not the MAX30100 : 0x%x\n",readI2CRegister(Board_MAX30100_ADDR,0xFF));
//		System_flush();
//	}

	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_MODE_CONFIGURATION, 0x02);	//enable HR only
	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_LED_CONFIGURATION, MAX30100_LED_CURR_50MA);	//set LED 50mA
	writeI2CRegister(Board_MAX30100_ADDR, 0x01, 0xE0);	//turn on interrupts

	//clearing FIFO write pointer, overflow counter and read pointer
//	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_WRITE_POINTER, 0x00);
//	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_OVERFLOW_COUNTER, 0x00);
//	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_READ_POINTER, 0x00);

	start_clock = Clock_getTicks();
	end_clock = Clock_getTicks();
	while(i < numValues){
	//while((end_clock-start_clock)/100000 < 5){
		while((readI2CRegister(Board_MAX30100_ADDR, 0x00) & 0x20) != 0x20){
			//check if hr data is ready
		}
		//fifo data is 16 bits so 4 reads is needed
		//first 16 bits is IR data, in our case, HR data
		//HRData[i] = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
		HRData[i] = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
		while(HRData[i] == 0){
			HRData[i] = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
		}

		Timestamp_getFreq(&frequency);

		if(i == 0){
			start_timestamp = Timestamp_get32();
		}

		Task_sleep(10000 / Clock_tickPeriod);						//adding this to extend measurement to 10 seconds

		end_timestamp = Timestamp_get32();
		end_clock = Clock_getTicks();

		//System_printf("%d\n" , HRData[i]);
		i++;
	}

	sampleData->heartRateData.rate_l = HRData[0] & 0xFF;
	sampleData->heartRateData.rate_h = HRData[0] >> 8;

	if(verbose_sensors){
		for(i = 0 ; i < numValues ; i++){
			System_printf("%d\n",HRData[i]);
		}

		System_printf("Timestamp diff: %d\n" , end_timestamp-start_timestamp);
		System_printf("Frequency %d\n", frequency.hi << 8 | frequency.lo);
		System_printf("Time elapsed: %d\n" , (end_timestamp-start_timestamp) / (frequency.hi << 8 | frequency.lo));

		System_printf("Clock diff: %d\n" , end_clock-start_clock);
		System_printf("Time elapsed: %d\n" , (end_clock-start_clock)/100000);

		System_flush();
	}

	return;
}

void getTimestamp(struct sampleData *sampleData){
	sampleData->timestamp = Timestamp_get32();
}

void makeSensorPacket(struct sampleData *sampleData){

	getAcceleration(sampleData);
	System_printf("A\n");
	System_flush();
	getTemp(sampleData);
	System_printf("A\n");
	System_flush();
	getHeartRate(sampleData);
	System_printf("A\n");
	System_flush();
	getTimestamp(sampleData);
	System_printf("A\n");
	System_flush();
	System_printf("x:%d y:%d z:%d\n", sampleData->accelerometerData.x ,sampleData->accelerometerData.y, sampleData->accelerometerData.z);
	System_printf("temp_obj low %x  high %x\n", sampleData->tempData.temp_l, sampleData->tempData.temp_h);
	System_printf("temp_amb low %x  high %x\n", sampleData->heartRateData.temp_l, sampleData->heartRateData.temp_h);
	System_printf("rate low %x  rate high %x\n", sampleData->heartRateData.rate_l, sampleData->heartRateData.rate_h);
	System_printf("timestamp %x\n", sampleData->timestamp);
	System_flush();
}

void testSensors(){
	struct sampleData sampleData;
	makeSensorPacket(&sampleData);
	//getTempNoPtr();
	//getTemp(&sampleData);

	System_printf("Tests done\n");
	System_flush();
}
