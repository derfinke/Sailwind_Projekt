/*
 * FRAM.h
 *
 *  Created on: Jun 18, 2023
 *      Author: nicof
 */

#ifndef FRAM_H_
#define FRAM_H_

uint32_t little_to_big_endian(uint32_t address);
uint8_t FRAM_init(void);
uint8_t FRAM_write(uint8_t *pStructToSave, const uint32_t startAddress, uint32_t sizeInByte);

#endif /* FRAM_H_ */