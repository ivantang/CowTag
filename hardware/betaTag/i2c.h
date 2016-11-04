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


Void echoFxn(UArg arg0, UArg arg1);

static void transferCallback(I2C_Handle handle, I2C_Transaction *transac, bool result);

static void writeI2CRegister(uint8_t destination[], uint8_t value[]);

Void initLIS3DH();
Void initMIKROE1362();


#endif /* I2C_H_ */
