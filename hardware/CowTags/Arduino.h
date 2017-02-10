/*
 * Arduino.h
 *
 *  Created on: Feb 7, 2017
 *      Author: ivan
 */

#ifndef ARDUINO_H_
#define ARDUINO_H_

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h> //i2c
#include <ti/drivers/UART.h> //i2c

#define ARDUINO_ADDR	0x08

void testArduino();

#endif /* ARDUINO_H_ */
