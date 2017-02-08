/*
 * i2c.h
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef SENSORS_H_
#define SENSORS_H_

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h> //i2c
#include <ti/drivers/UART.h> //i2c

/*initialize node task and create TI-RTOS objects*/
void Sensors_init(void);

/*LIS3DH ACCELEROMETER ADDRESSES*/
#define     Board_LIS3DH_ADDR		 0x18 // 0011001
#define 	LIS3DH_REG_STATUS1       0x07
#define 	LIS3DH_REG_OUTADC1_L     0x08
#define 	LIS3DH_REG_OUTADC1_H     0x09
#define		LIS3DH_REG_OUTADC2_L     0x0A
#define 	LIS3DH_REG_OUTADC2_H     0x0B
#define		LIS3DH_REG_OUTADC3_L     0x0C
#define 	LIS3DH_REG_OUTADC3_H     0x0D
#define		LIS3DH_REG_INTCOUNT      0x0E
#define 	LIS3DH_REG_WHOAMI        0x0F
#define 	LIS3DH_REG_TEMPCFG       0x1F
#define 	LIS3DH_REG_CTRL1         0x20
#define 	LIS3DH_REG_CTRL2         0x21
#define 	LIS3DH_REG_CTRL3         0x22
#define 	LIS3DH_REG_CTRL4         0x23
#define 	LIS3DH_REG_CTRL5         0x24
#define 	LIS3DH_REG_CTRL6         0x25
#define 	LIS3DH_REG_REFERENCE     0x26
#define 	LIS3DH_REG_STATUS2       0x27
#define 	LIS3DH_REG_OUT_X_L       0x28
#define 	LIS3DH_REG_OUT_X_H       0x29
#define 	LIS3DH_REG_OUT_Y_L       0x2A
#define 	LIS3DH_REG_OUT_Y_H       0x2B
#define 	LIS3DH_REG_OUT_Z_L       0x2C
#define 	LIS3DH_REG_OUT_Z_H       0x2D
#define 	LIS3DH_REG_FIFOCTRL      0x2E
#define 	LIS3DH_REG_FIFOSRC       0x2F
#define 	LIS3DH_REG_INT1CFG       0x30
#define 	LIS3DH_REG_INT1SRC       0x31
#define 	LIS3DH_REG_INT1THS       0x32
#define 	LIS3DH_REG_INT1DUR       0x33
#define 	LIS3DH_REG_CLICKCFG      0x38
#define 	LIS3DH_REG_CLICKSRC      0x39
#define 	LIS3DH_REG_CLICKTHS      0x3A
#define 	LIS3DH_REG_TIMELIMIT     0x3B
#define 	LIS3DH_REG_TIMELATENCY   0x3C
#define 	LIS3DH_REG_TIMEWINDOW    0x3D

/*MIKROE1362 IR TEMP ADDRESSES*/
#define	Board_MIKROE1362_ADDR	0x5A
// RAM
#define MLX90614_RAWIR1 		0x04
#define MLX90614_RAWIR2 		0x05
#define MLX90614_TA 			0x06	//ambient temp
#define MLX90614_TOBJ1 			0x07	//object temp
#define MLX90614_TOBJ2 			0x08
// EEPROM
#define MLX90614_TOMAX 			0x20
#define MLX90614_TOMIN 			0x21
#define MLX90614_PWMCTRL	    0x22
#define MLX90614_TARANGE 		0x23
#define MLX90614_EMISS			0x24
#define MLX90614_CONFIG 		0x25
#define MLX90614_ADDR 			0x0E
#define MLX90614_ID1 			0x3C
#define MLX90614_ID2 			0x3D
#define MLX90614_ID3 			0x3E
#define MLX90614_ID4 			0x3F

/*MAX30100 HEART RATE ADDRESSES*/
#define Board_MAX30100_ADDR     0x57
//FIFO CONTROL and DATA REGISTERS
#define MAX30100_REG_FIFO_WRITE_POINTER         0x02
#define MAX30100_REG_FIFO_OVERFLOW_COUNTER      0x03
#define MAX30100_REG_FIFO_READ_POINTER          0x04
#define MAX30100_REG_FIFO_DATA                  0x05  // Burst read does not autoincrement addr

// Mode Configuration register
#define MAX30100_REG_MODE_CONFIGURATION         0x06

//Defaults
#define	MAX30100_MODE_HRONLY			0x02	//default mode
#define MAX30100_REG_SPO2_CONFIGURATION 0x07
#define MAX30100_REG_LED_CONFIGURATION  0x09

#define	MAX30100_MODE_SPO2_HR			0x03	//default LEDPUlseWidth
#define MAX30100_SAMPRATE_100HZ			0x01	//default Samplerate
#define	MAX30100_SPC_PW_1600US_16BITS	0x03	//default LEDPulseWidth
#define	MAX30100_LED_CURR_50MA			0x0f	//default LEDcurrent
#define MAX30100_SPC_SPO2_HI_RES_EN     (1 << 6)

struct temperatureData {
	uint16_t temp_l;
	uint16_t temp_h;
	uint32_t timestamp;
};

struct accelerationData {
	uint16_t x;
	uint16_t y;
	uint16_t z;
	uint32_t timestamp;
};

struct heartrateData {
	uint16_t temp_l;
	uint16_t temp_h;
	uint16_t rate_l;
	uint16_t rate_h;
	uint32_t timestamp;
};

//void echoFxn(UArg arg0, UArg arg1);
struct accelerationData  getAcceleration();
struct temperatureData getObjTemp();
struct heartrateData  getHeartRate();
void testSensors();

#endif /* SENSORS_H_ */
