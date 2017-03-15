/*
 * IIC.h
 *
 *  Created on: Nov 24, 2016
 *      Author: Ivan
 */

#ifndef IIC_H_
#define IIC_H_


#define retryI2CCount	1

/***** Includes *****/
#include <stdint.h>

/***** Prototypes *****/
uint8_t readI2CEEPROM(uint8_t slaveaddr, uint8_t addrHigh, uint8_t addrLow);
uint8_t readI2CRegister100kHz(uint8_t board_address, uint8_t address);
void transferCallback(I2C_Handle handle, I2C_Transaction *transac, bool result);
void writeI2CEEPROM(uint8_t slaveAddr, uint8_t bytes[]);
void writeI2CRegister(uint8_t board_address, uint8_t destination, uint8_t value);
void writeI2CRegisters(int8_t board_address, uint8_t destination[], uint8_t value[]);
uint32_t readI2CWord100kHz(uint8_t board_address, uint8_t address);
uint8_t readI2CRegister(uint8_t board_address, uint8_t address);
uint8_t readI2CRegister100kHz(uint8_t board_address, uint8_t address);
uint8_t readI2CEEPROM(uint8_t slaveaddr, uint8_t addrHigh, uint8_t addrLow);

#endif /* IIC_H_ */
