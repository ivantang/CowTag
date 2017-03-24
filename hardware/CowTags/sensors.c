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
#include <stdint.h>
#include <assert.h>

#include <sensors.h>
#include <IIC.h>
#include "bootTimestamp.h"
#include "global_cfg.h"

PIN_Config BoardGpioInitialTable[] = {
//		Board_I2C0_SDA0 | PIN_GPIO_LOW,
//		Board_I2C0_SCL0	| PIN_GPIO_HIGH,
		PIN_TERMINATE
};

PIN_State pinState;

/**constants*/
#define TASKSTACKSIZE		8192	//i2c

/**structures*/
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*function definition */
void Sensors_test(void) {
	//PIN_init(BoardGpioInitialTable);
	Task_Params taskParams;

	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.stack = &task0Stack;
	Task_construct(&task0Struct, (Task_FuncPtr)testSensors, &taskParams, NULL);
}


void getAcceleration(struct sampleData *sampleData) {
	//if (verbose_sensors)System_printf("\n\nwhoamI: 0x%x \n", readI2CRegister(Board_LIS3DH_ADDR, 15)); //should read 0x33
	//System_flush();

	unsigned int	i;
	char accelerometer_state = 0x00;

	struct sampleData accelerationData[30];

	switch(ACCELEROMETER_SAMPLE_RATE_HZ) {
			case 1:
				// 1 specifies 1Hz, the 7 enables all axes
				accelerometer_state = 0x17;
				break;
			case 10:
				// 2 specifies 10Hz, the 7 enables all axes
				accelerometer_state = 0x27;
				break;
			case 25:
				// etc
				accelerometer_state = 0x37;
				break;
			case 50:
				accelerometer_state = 0x47;
				break;
			case 100:
				accelerometer_state = 0x57;
				break;
			case 200:
				accelerometer_state = 0x67;
				break;
			case 400:
				accelerometer_state = 0x77;
				break;
			default:
				// fail if we get anything else.
				assert(0);
		}
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL1, accelerometer_state);    //all axes , HR/normal mode, 10Hz
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL4, 0x0C);	//high res and BDU and self test off +/-2g
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL3, 0x10);    //DRDY on INT1
    //writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_TEMPCFG, 0x80);    //enable adcs
    //writeI2C(Board_LIS3DH_ADDR, LIS3DH_REG_OUT_X_L | 0x80);    //enable auto increment

	//check if registers have correct values
	System_printf("%x\n", readI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL1));
	System_printf("%x\n", readI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL4));
	System_printf("%x\n", readI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL3));


    //polling status register to check for new set of data
    for(i = 0 ; i < 30 ;){
    	if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) & 0x8) >> 3 == 1 ){
    		if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) >> 7) == 1 ){
    			accelerationData[i].accelerometerData.x = readI2CRegister(Board_LIS3DH_ADDR,0x28) | (readI2CRegister(Board_LIS3DH_ADDR,0x29) << 8);
    			accelerationData[i].accelerometerData.y = readI2CRegister(Board_LIS3DH_ADDR,0x2A) | (readI2CRegister(Board_LIS3DH_ADDR,0x2B) << 8) ;
    			accelerationData[i].accelerometerData.z = readI2CRegister(Board_LIS3DH_ADDR,0x2C) | (readI2CRegister(Board_LIS3DH_ADDR,0x2D) << 8) ;

    			sampleData->accelerometerData.x = accelerationData[i].accelerometerData.x;
    			sampleData->accelerometerData.y = accelerationData[i].accelerometerData.y;
    			sampleData->accelerometerData.z = accelerationData[i].accelerometerData.z;

    			if(verbose_sensors) System_printf("x:%i y:%i z:%i\n", 	sampleData->accelerometerData.x ,
    																	sampleData->accelerometerData.y,
																		sampleData->accelerometerData.z);
    			System_flush();
    			i++;
    		}
    	}
    }

    //write to file
    for(i = 0 ; i < 30 ;i++){
    	System_printf("x:%3d y:%3d z:%3d\n", 	accelerationData[i].accelerometerData.x ,
    										accelerationData[i].accelerometerData.y,
											accelerationData[i].accelerometerData.z);
    	accelerationData[i].packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
    	//file_printSampleData(accelerationData[i]);
    }


    System_flush();

    return;
}

void getTemp(struct sampleData *sampleData) {
	uint16_t temp_obj;
	uint16_t temp_amb;

	temp_obj =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x07)*0.02 - 273.15;
	sampleData->tempData.temp_l = temp_obj & 0xFF;
	sampleData->tempData.temp_h = temp_obj >> 8;

	temp_amb =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x06)*0.02 - 273.15;
	sampleData->heartRateData.temp_l = temp_amb & 0xFF;
	sampleData->heartRateData.temp_h = temp_amb >> 8;

	if (verbose_sensors) {
		System_printf("temp_obj %d\n", temp_obj);
	}

	return;
}

void getTempNoPtr() {
	uint16_t temp_obj;
	uint16_t temp_amb;


	temp_obj =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x07)*0.02 - 273.15;
	//sampleData->tempData.temp_l = temp_obj & 0xFF;
	//sampleData->tempData.temp_h = temp_obj >> 8;


	temp_amb =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x06)*0.02 - 273.15;
	//sampleData->heartRateData.temp_l = temp_amb & 0xFF;
	//sampleData->heartRateData.temp_h = temp_amb >> 8;

	if (verbose_sensors) {
		System_printf("temp_obj %d\n", temp_obj);
		System_printf("temp_obj %d\n", temp_amb);
		System_flush();
	}

	return;
}

void getHeartRateContinuous(struct sampleData *sampleData) {
	int i = 0;
	int j = 0;
	uint16_t HRData = 0;

	System_printf("Getting heart rate..\n");
	System_flush();

	//check if device is connected
	if(readI2CRegister(Board_MAX30100_ADDR,0xFF) != 0x11){
		System_printf("Hardware is not the MAX30100 : 0x%x\n",readI2CRegister(Board_MAX30100_ADDR,0xFF));
		System_flush();
	}


	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_MODE_CONFIGURATION, 0x02);	//enable HR only

	uint8_t previous = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_SPO2_CONFIGURATION);
	writeI2CRegister(Board_MAX30100_ADDR,
					 MAX30100_REG_SPO2_CONFIGURATION,
					 previous & 0xA0 | 0x47);	//SPO2 cofig reg

	writeI2CRegister(Board_MAX30100_ADDR,
					 MAX30100_REG_LED_CONFIGURATION,
					 0x0f);	//set LED 50mA

	previous = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_CONFIGURATION);
	writeI2CRegister(Board_MAX30100_ADDR,
				 	 MAX30100_REG_INTERRUPT_CONFIGURATION,
					 previous & 0x0A | 0xA0);	//turn on interrupts

	uint32_t tempval;

	while(1){
		//clear FIFO PTR
		writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_WRITE_POINTER, 0x00);
		writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_READ_POINTER, 0x00);

		//check if hr data is ready
		//tempval = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_STATUS);

		while(!tempval){
			tempval = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_STATUS);
			tempval &= MAX30100_HRDATA_READY;
			//System_printf(".");
			//System_flush();
			if(tempval &= MAX30100_DATA_FULL){
				System_printf("IM FULL\n");
				System_flush();
			}
		}

		HRData = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);

		//fifo data is 16 bits so 4 reads is needed
		//first 16 bits is IR data, in our case, HR data
		//filter out 0 measurements
		while(HRData == 0){
			HRData = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
		}

		/*if(HRData[i] < 40000 || HRData[i] > 60000){
			HRData = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
		}*/

		System_printf("HR %d\n",HRData);


		//next 16 bits is useless data red led data
		//RedData[i] = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
	}

	return;
}

void getHeartRate(struct sampleData* sampleData) {
	int i = 0;
	int j = 0;
	uint16_t numValues = 50;
	//uint16_t RedData[250];
	//int Derivative[75];
	struct sampleData HRData[50];

	System_printf("Getting heart rate..\n");
	System_flush();

	//check if device is connected
	if(readI2CRegister(Board_MAX30100_ADDR,0xFF) != 0x11){
		System_printf("Hardware is not the MAX30100 : 0x%x\n",readI2CRegister(Board_MAX30100_ADDR,0xFF));
		System_flush();
	}

	writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_MODE_CONFIGURATION, 0x02);	//enable HR only

//	uint8_t previous = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_SPO2_CONFIGURATION);
//	writeI2CRegister(Board_MAX30100_ADDR,
//					 MAX30100_REG_SPO2_CONFIGURATION,
//					 previous & 0xA0 | 0x47);	//SPO2 cofig reg

	// previous holds the current state of the heartrate configuration
	uint8_t previous = readI2CRegister(Board_MAX30100_ADDR,
	                                   MAX30100_REG_SPO2_CONFIGURATION);

	// Now we take that previous configuration and "and" it with 0xA0 to keep only
	// the information that we want to keep from the previous setting. Then "or"
	// it with 0x43 to set our own global settings
	// The register is now set like so: x1x0 0011 (x denotes, does not matter)
	// This sets high res on ------------^     ^^
	// and LED Pulse width to 1600 us ---------^^
	// See pg16: https://datasheets.maximintegrated.com/en/ds/MAX30100.pdf
	char heartrate_config = previous & 0xA0 | 0x43;
	// Now to set the sample rate... the register we are using has the bits
	// corresponding to the sample rate at the locations marked by "^"
	// bbbb bbbb
	//    ^ ^^
	// Since it is split down the middle of a hex value, the following numbers we
	// are assigning are not intuitive
	switch(HEARTRATE_SAMPLE_RATE_HZ) {
		case 50:
			// 0x00 specifies 50Hz. We or it so that our previous settings stay, we
			// are just adding these settings for the heartrate
			heartrate_config |= 0x00;
			break;
		case 100:
			// 0x04 specifies 100Hz
			heartrate_config |= 0x04;
			break;
		case 167:
			// etc
			heartrate_config |= 0x08;
			break;
		case 200:
			heartrate_config |= 0x0C;
			break;
		case 400:
			heartrate_config |= 0x10;
			break;
		case 600:
			heartrate_config |= 0x14;
			break;
		case 800:
			heartrate_config |= 0x18;
			break;
		case 1000:
			heartrate_config |= 0x1C;
			break;
		default:
			// fail if anything else
			assert(0);
	}

	writeI2CRegister(Board_MAX30100_ADDR,
					 MAX30100_REG_SPO2_CONFIGURATION,
					 heartrate_config);	//SPO2 cofig reg

	writeI2CRegister(Board_MAX30100_ADDR,
					 MAX30100_REG_LED_CONFIGURATION,
					 0x0f);	//set LED 50mA

	previous = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_CONFIGURATION);
	writeI2CRegister(Board_MAX30100_ADDR,
				 	 MAX30100_REG_INTERRUPT_CONFIGURATION,
					 previous & 0x0A | 0xA0);	//turn on interrupts

	uint32_t tempval;

//	check if registers have correct values
//	System_printf("%x\n", readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_SPO2_CONFIGURATION));
//	System_printf("%x\n", readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_CONFIGURATION));
//	System_printf("%x\n", readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_LED_CONFIGURATION));
//	System_printf("%x\n", readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_MODE_CONFIGURATION));
//	System_flush();

	while(i < numValues){
		//clear FIFO PTR
		writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_WRITE_POINTER, 0x00);
		writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_READ_POINTER, 0x00);

		//check if hr data is ready
		//tempval = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_STATUS);

		while(!tempval){
			tempval = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_STATUS);
			tempval &= MAX30100_HRDATA_READY;
			//System_printf(".");
			//System_flush();
			if(tempval &= MAX30100_DATA_FULL){
				System_printf("IM FULL\n");
				System_flush();
			}
		}


		//fifo data is 16 bits so 4 reads is needed
		//first 16 bits is IR data, in our case, HR data
		HRData[i].heartRateData.rate_h = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
		HRData[i].heartRateData.rate_l = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_DATA);
		readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_DATA);
		//filter out 0 measurements
		while(HRData[i].heartRateData.rate_h << 8 + HRData[i].heartRateData.rate_l == 0){
			HRData[i].heartRateData.rate_h = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
			HRData[i].heartRateData.rate_l = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_DATA);
			readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_DATA);
		}

		/*if(HRData[i] < 40000 || HRData[i] > 60000){
			if(i != 0)
			HRData[i] = HRData[i-1];
		}*/

		//next 16 bits is useless data red led data
		//RedData[i] = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);

		//if(i!=0) Derivative[i] = (int)(HRData[i] - HRData[i-1]);

		i++;
	}

	if (verbose_sensors) {
		for (i = 0 ; i < numValues ; i++) {
			System_printf("%i\n",HRData[i].heartRateData.rate_h << 8 + HRData[i].heartRateData.rate_l);

			HRData[i].packetType = RADIO_PACKET_TYPE_SENSOR_PACKET;
	    	file_printSampleData(HRData[i]);
		}

		System_flush();
	}

	return;
}


void getTimestamp(struct sampleData *sampleData) {
	sampleData->timestamp = TrueTimestamp();
}

void makeSensorPacket(struct sampleData *sampleData) {

//	getAcceleration(sampleData);

//	getTemp(sampleData);

	getHeartRate(sampleData);

	//getTimestamp(sampleData);

	/*System_printf(							"TemperatureCowData = %i ,"
											"AmbientTemperatureData = %i ,"
											"InfraredData = %i\n"
											"AccelerometerData= x=%i, y=%i, z=%i\n"
											"Timestamp = %i\n",
											sampleData->tempData.temp_h << 8 | sampleData->tempData.temp_l,
											sampleData->heartRateData.temp_h << 8 | sampleData->heartRateData.temp_l,
											sampleData->heartRateData.rate_h << 8 | sampleData->heartRateData.rate_l,
											sampleData->accelerometerData.x,
											sampleData->accelerometerData.y,
											sampleData->accelerometerData.z,
											sampleData->timestamp);
*/
	System_flush();
}

void testSensors() {
	struct sampleData sampleData;

	makeSensorPacket(&sampleData);

	System_printf("Tests done\n");
	System_flush();


//	getHeartRate(&sampleData);
}
