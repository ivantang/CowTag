/*
 * ArduinoCom.h
 *
 *  Created on: Feb 10, 2017
 *      Author: Erik-PC
 */

#ifndef ARDUINOCOM_H_
#define ARDUINOCOM_H_

/***** Includes *****/
#include <stdint.h>

/***** Prototypes *****/
void writeI2CArduino(uint8_t slaveAddr, uint8_t bytes[]);

#endif /* ARDUINOCOM_H_ */
