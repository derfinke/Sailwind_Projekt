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

static uint8_t FRAM_read_status_register(uint8_t *pRegisterStatus);
static uint8_t FRAM_write_enable(void);


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
		printf("Failed setting WREN!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
	return FRAM_OK;
}

uint8_t FRAM_init(void)
{
	uint8_t registerStatus;

	printf("Starting FRAM init\r\n");

	FRAM_write_enable();
	if((FRAM_write_enable() || FRAM_read_status_register(&registerStatus)) != FRAM_OK)
	{
		printf("FRAM init failed!\r\n");
		return FRAM_ERROR;
	}

	if(registerStatus != 2U)
	{
		printf("Failed setting WREN!\r\n");
		return FRAM_ERROR;
	}
	printf("FRAM init completed\r\n");

	return FRAM_OK;
}

static uint8_t FRAM_read_status_register(uint8_t *pRegisterStatus)
{
	HAL_SPI_StateTypeDef spiStatus;

	assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
	spiSTatus = HAL_SPI_TransmitReceive(&hspi4, RDSR, *pRegisterStatus, 2U, SPI_HAL_TIMEOUT);

	if(spiStatus != HAL_OK)
	{
		printf("Failed reading status register!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
	return FRAM_OK;
}
