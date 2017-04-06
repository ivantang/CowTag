/*
 * serialize.h
 *
 * This driver converts data samples from C structs to
 * raw byte data. This raw data can be sent between tags,
 * or stored into the eeprom with this format.
 *
 *  Created on: Feb 6, 2017
 *      Author: champ
 */

#ifndef SERIALIZE_H_
#define SERIALIZE_H_

void serializePacket(struct sampleData *data, uint8_t *buffer);
void unserializePacket(struct sampleData *data, uint8_t *buffer);

#endif /* SERIALIZE_H_ */
