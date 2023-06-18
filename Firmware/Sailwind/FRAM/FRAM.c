/*
 * FRAM.c
 *
 *  Created on: 18 Jun 2023
 *      Author: nicof
 */

#include "FRAM.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "data_logger.h"

#define WRSR 1
#define WRITE 2
#define READ 3
#define WRDI 4
#define RDSR 5
#define WREN 6
#define SPI_HAL_TIMEOUT 5U
#define FRAM_OK 0U
#define FRAM_ERROR 1U

static uint8_t FRAM_write(uint8_t *pStructToSave, const uint32_t startAddress, uint32_t sizeInByte)
{
	assert(pStructToSave != 0);
	assert(sizeInByte != 0);


}

static uint8_t FRAM_write_enable(void)
{
	HAL_SPI_StateTypeDef spiStatus;

	assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
	spiSTatus = HAL_SPI_Transmit(&hspi4, WREN, 1U, SPI_HAL_TIMEOUT);

	if(spiStatus != HAL_OK)
	{
		printf("Failed setting WREN!");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
	return FRAM_OK;
}

