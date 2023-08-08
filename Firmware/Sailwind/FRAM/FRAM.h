/*
 * FRAM.h
 *
 *  Created on: Jun 18, 2023
 *      Author: nicof
 */

#ifndef FRAM_H_
#define FRAM_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi4;

/**
* @brief initialize FRAM
* Configures FRAM to be ready to write
* @param none
* @retval FRAM status
*/
uint8_t FRAM_init(void);

/**
* @brief write to FRAM
* @param pStructToSave: pointer to structer that should be saved
* @param startAddress: address where struct is saved to
* @param sizeInByte: size of struct to be saved
* @retval FRAM status
*/
uint8_t FRAM_write(uint8_t *pStructToSave, const uint32_t startAddress, uint32_t sizeInByte);

/**
* @brief write to FRAM
* @param pData: pointer to buffer where read data is saved to
* @param startAddress: address where data is read from
* @param sizeInByte: size of data to be read
* @retval FRAM status
*/
uint8_t FRAM_read(uint32_t startAddress, uint8_t *pData, uint32_t sizeInByte);

#endif /* FRAM_H_ */
