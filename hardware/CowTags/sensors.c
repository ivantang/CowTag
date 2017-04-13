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
	PIN_TERMINATE
};

PIN_State pinState;

/**constants*/
#define TASKSTACKSIZE		0

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
	unsigned int	i;
	unsigned samplesToGet = 100;
	char accelerometer_state = 0x00;

	struct sampleData accelerationData[1];

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
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL4, 0x08);	//high res and BDU and self test off +/-2g
    writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_CTRL3, 0x10);    //DRDY on INT1
    //writeI2CRegister(Board_LIS3DH_ADDR, LIS3DH_REG_TEMPCFG, 0x80);    //enable adcs
    //writeI2C(Board_LIS3DH_ADDR, LIS3DH_REG_OUT_X_L | 0x80);    //enable auto increment

    //polling status register to check for new set of data
    //if(!grabOnlyOne){
		for(i = 0 ; i < samplesToGet ;){
			if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) & 0x8) >> 3 == 1 ){
				if( (readI2CRegister(Board_LIS3DH_ADDR,0x27) >> 7) == 1 ){
					accelerationData[i].accelerometerData.x_h = readI2CRegister(Board_LIS3DH_ADDR,0x28);
					accelerationData[i].accelerometerData.x_l = readI2CRegister(Board_LIS3DH_ADDR,0x29);
					accelerationData[i].accelerometerData.y_h = readI2CRegister(Board_LIS3DH_ADDR,0x2A);
					accelerationData[i].accelerometerData.y_l = readI2CRegister(Board_LIS3DH_ADDR,0x2B);
					accelerationData[i].accelerometerData.z_h = readI2CRegister(Board_LIS3DH_ADDR,0x2C);
					accelerationData[i].accelerometerData.z_l = readI2CRegister(Board_LIS3DH_ADDR,0x2D);

					sampleData->accelerometerData.x_h = accelerationData[i].accelerometerData.x_h;
					sampleData->accelerometerData.x_l = accelerationData[i].accelerometerData.x_l;
					sampleData->accelerometerData.y_h = accelerationData[i].accelerometerData.y_h;
					sampleData->accelerometerData.y_l = accelerationData[i].accelerometerData.y_l;
					sampleData->accelerometerData.z_h = accelerationData[i].accelerometerData.z_h;
					sampleData->accelerometerData.z_l = accelerationData[i].accelerometerData.z_l;

			    	System_flush();

					break;
				}
			}
		}
    //}

    //write to file
    //for(i = 0 ; i < samplesToGet ;i++){
//    	System_printf("x:%i y:%i z:%i\n", 	sampleData->accelerometerData.x_h << 8 | sampleData->accelerometerData.x_l,
//											sampleData->accelerometerData.y_h << 8 | sampleData->accelerometerData.y_l,
//											sampleData->accelerometerData.z_h << 8 | sampleData->accelerometerData.z_l);
    	if(verbose_beta_log){
			accelerationData[i].packetType = RADIO_PACKET_TYPE_ACCEL_PACKET;
			file_printSampleData(accelerationData[i]);
    	}
    //}


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

	/*if (verbose_sensors) {
		System_printf("temp_obj %d\n", temp_obj);
	}*/

	if (verbose_beta_log){
		sampleData->packetType = RADIO_PACKET_TYPE_TEMP_PACKET;
		file_printSampleData(*sampleData);
	}

	return;
}

void getTempNoPtr() {
	uint16_t temp_obj;
	uint16_t temp_amb;


	temp_obj =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x07)*0.02 - 273.15;

	temp_amb =  readI2CWord100kHz(Board_MIKROE1362_ADDR,0x06)*0.02 - 273.15;

	if (verbose_sensors) {
		System_printf("temp_obj %d\n", temp_obj);
		System_printf("temp_obj %d\n", temp_amb);
		System_flush();
	}

	return;
}


void getHeartRate(struct sampleData* sampleData) {
	int i = 0;
	uint16_t numValues = 500;
	//uint16_t RedData[250];
	//int Derivative[75];
	uint8_t rate_l[500];
	uint8_t rate_h[500];

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

	if(!grabOnlyOne){
		while(i < numValues){
			//clear FIFO PTR
			//writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_WRITE_POINTER, 0x00);
			//writeI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_READ_POINTER, 0x00);

			//check if hr data is ready
			while(!tempval){
				tempval = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_STATUS);
				tempval &= MAX30100_HRDATA_READY;
			}

			//fifo data is 16 bits so 4 reads is needed
			//first 16 bits is IR data, in our case, HR data
			rate_h[i] = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
			rate_l[i] = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_DATA);

			while(!tempval){
				tempval = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_INTERRUPT_STATUS);
				tempval &= 0x10;
			}

			readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_DATA);
			//filter out 0 measurements
			/*while( (rate_h[i] << 8 + rate_l[i] == 0)){
				rate_h[i] = readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA);
				rate_l[i] = readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_DATA);
				//readI2CRegister(Board_MAX30100_ADDR,MAX30100_REG_FIFO_DATA)<<8 + readI2CRegister(Board_MAX30100_ADDR, MAX30100_REG_FIFO_DATA);
			}*/

			break;
		}
	}

	//heart rate algorithm
	uint16_t peak = 0;
	unsigned actualHeartRate = 0;

	for (i = 0; i < numValues; i++) {
		if( (rate_h[i] << 8 + rate_l[i]) < peak){
			actualHeartRate++;
			peak = 0;
		} else {
			if(rate_h[i] << 8 + rate_l[i] != 0) peak = rate_h[i] << 8 + rate_l[i];
			//System_printf("HR %i\n", rate_h[i] << 8 + rate_l[i]);
		}
	}

	sampleData->heartRateData.rate_l = actualHeartRate;

	return;
}


void getTimestamp(struct sampleData *sampleData) {
	sampleData->timestamp = TrueTimestamp();
}

void makeSensorPacket(struct sampleData *sampleData) {
	getTemp(sampleData);

	getAcceleration(sampleData);

	getHeartRate(sampleData);

	if(verbose_sensors)
	System_printf(							"TemperatureCowData = %i ,"
											"AmbientTemperatureData = %i ,"
											"Heartrate = %i\n"
											"AccelerometerData= x=%i, y=%i, z=%i\n"
											"Timestamp = %i\n",
											sampleData->tempData.temp_h << 8 | sampleData->tempData.temp_l,
											sampleData->heartRateData.temp_h << 8 | sampleData->heartRateData.temp_l,
											sampleData->heartRateData.rate_l,
											sampleData->accelerometerData.x_h << 8 | sampleData->accelerometerData.x_l,
											sampleData->accelerometerData.y_h << 8 | sampleData->accelerometerData.y_l,
											sampleData->accelerometerData.z_h << 8 | sampleData->accelerometerData.z_l,
											sampleData->timestamp);

}

void testSensors() {
	struct sampleData sampleData;

	while(1){
		makeSensorPacket(&sampleData);
	}
}
