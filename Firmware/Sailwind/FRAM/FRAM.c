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

#define WRSR 1
#define WRITE 2
#define READ 3
#define WRDI 4
#define RDSR 5
#define WREN 6
#define SPI_HAL_TIMEOUT 5U
#define FRAM_OK 0U
#define FRAM_ERROR 1U
#define WEL_SET 2U
#define STATUS_REGISTER_BUFFER_SIZE 2U

static uint8_t FRAM_read_status_register(void);


uint8_t FRAM_write(uint8_t *pStructToSave, const uint32_t startAddress, uint32_t sizeInByte)
{
	HAL_SPI_StateTypeDef spiStatus;
	uint8_t registerStatus;
	uint32_t address;

	assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);
	assert(pStructToSave != 0);
	assert(sizeInByte != 0);

	if(FRAM_read_status_register() != WEL_SET)
	{
		printf("WEL not set!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	address = little_to_big_endian(startAddress);

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
	spiStatus = HAL_SPI_Transmit(&hspi4, WRITE, 1U, SPI_HAL_TIMEOUT);
	if(spiStatus != HAL_OK)
	{
		printf("Failed sending Write instruction!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	spiStatus = HAL_SPI_Transmit(&hspi4, (uint8_t*)&address, sizeof(address), SPI_HAL_TIMEOUT);
	if(spiStatus != HAL_OK)
	{
		printf("Failed sending address!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	hal_SPI_status= HAL_SPI_Transmit(&hspi3, struct_to_save, size_in_bytes, SPI_HAL_TIMEOUT);
	if (hal_SPI_status != HAL_OK)
	{
		printf("Failed sending data to be saved!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);

	return FRAM_OK;

}

uint8_t FRAM_read(uint32_t startAddress, uint8_t *pData, uint32_t sizeInByte)
{
	HAL_SPI_StateTypeDef spiStatus;
	uint8_t tx_dummy[sizeInByte];
	uint32_t address;

	assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);
	assert(pData != 0);
	assert(sizeInByte != 0);

	address = little_to_big_endian(startAddress);

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);

	spiSTatus = HAL_SPI_Transmit(&hspi4, READ, 1U, SPI_HAL_TIMEOUT);
	if(spiStatus != HAL_OK)
	{
		printf("Failed sending READ!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	spiStatus = HAL_SPI_Transmit(&hspi4, (uint8_t*)&address, sizeof(address), SPI_HAL_TIMEOUT);
	if(spiStatus != HAL_OK)
	{
		printf("Failed sending address!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	spiStatus = HAL_SPI_Transmit(&hspi4, tx_dummy[0], 1U, SPI_HAL_TIMEOUT);
	if(spiStatus != HAL_OK)
	{
		printf("Failed sending dummy byte!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	spiStatus = HAL_SPI_TransmitReceive(&hspi4, tx_dummy, pData, sizeInByte, SPI_HAL_TIMEOUT);
	if(spiStatus != HAL_OK)
	{
		printf("Failed receiving data!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	HAL_Delay(1);

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);

	return FRAM_OK;
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

uint8_t FRAM_read_status_register()
{
	HAL_SPI_StateTypeDef spiStatus;
	uint8_t statusRegTx[STATUS_REGISTER_BUFFER_SIZE];
	uint8_t statusRegRx[STATUS_REGISTER_BUFFER_SIZE];

	assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);

	statusRegTx[0] = RDSR;

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
	spiSTatus = HAL_SPI_TransmitReceive(&hspi4, statusRegTx, statusRegRx, STATUS_REGISTER_BUFFER_SIZE, SPI_HAL_TIMEOUT);

	if(spiStatus != HAL_OK)
	{
		printf("Failed reading status register!\r\n");
	}

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
	return statusRegRx[1];
}
