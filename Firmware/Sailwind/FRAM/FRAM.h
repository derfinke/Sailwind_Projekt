/*
 * FRAM.h
 *
 *  Created on: Jun 18, 2023
 *      Author: nicof
 */

#ifndef FRAM_H_
#define FRAM_H_

#include <stdint.h>

uint32_t little_to_big_endian(uint16_t address);
uint8_t FRAM_init(void);
uint8_t FRAM_write(uint8_t *pStructToSave, const uint16_t startAddress, uint16_t sizeInByte);
uint8_t FRAM_read(uint16_t startAddress, uint8_t *pData, const uint16_t sizeInByte);

#endif /* FRAM_H_ */
