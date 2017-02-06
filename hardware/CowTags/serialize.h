/*
 * serialize.h
 *
 *  Created on: Feb 6, 2017
 *      Author: champ
 */

#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#include "radioProtocol.h"


void serializePacket(struct sensorPacket *packet, uint8_t *buffer);
void unserializePacket(struct sensorPacket *packet, uint8_t *buffer);

#endif /* SERIALIZE_H_ */
