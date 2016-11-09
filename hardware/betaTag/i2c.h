/*
 * i2c.h
 *
 *  Created on: Nov 4, 2016
 *      Author: annik
 */

#ifndef I2C_H_
#define I2C_H_

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h> //i2c
#include <ti/drivers/UART.h> //i2c

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

Void echoFxn(UArg arg0, UArg arg1);

static void transferCallback(I2C_Handle handle, I2C_Transaction *transac, bool result);

static void writeI2CRegister(uint8_t destination[], uint8_t value[]);

Void initLIS3DH();
Void initMIKROE1362();


#endif /* I2C_H_ */
