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
#include "main.h"

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


uint8_t FRAM_write(uint8_t *pStructToSave, const uint16_t startAddress, uint16_t sizeInByte)
{
	HAL_SPI_StateTypeDef spiStatus;
	uint16_t address;

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

	spiStatus = HAL_SPI_Transmit(&hspi4, pStructToSave, sizeInByte, SPI_HAL_TIMEOUT);
	if (spiStatus != HAL_OK)
	{
		printf("Failed sending data to be saved!\r\n");
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
		return FRAM_ERROR;
	}

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);

	return FRAM_OK;

}

uint8_t FRAM_read(uint16_t startAddress, uint8_t *pData, const uint16_t sizeInByte)
{
	HAL_SPI_StateTypeDef spiStatus;
	uint8_t tx_dummy[sizeInByte];
	uint16_t address;

	assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);
	assert(pData != 0);
	assert(sizeInByte != 0);

	address = little_to_big_endian(startAddress);

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);

	spiStatus = HAL_SPI_Transmit(&hspi4, READ, 1U, SPI_HAL_TIMEOUT);
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
	spiStatus = HAL_SPI_Transmit(&hspi4, WREN, 1U, SPI_HAL_TIMEOUT);

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

	if((FRAM_write_enable() || FRAM_read_status_register()) != FRAM_OK)
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

uint8_t FRAM_read_status_register(void)
{
	HAL_SPI_StateTypeDef spiStatus;
	uint8_t statusRegTx[STATUS_REGISTER_BUFFER_SIZE];
	uint8_t statusRegRx[STATUS_REGISTER_BUFFER_SIZE];

	assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);

	statusRegTx[0] = RDSR;

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
	spiStatus = HAL_SPI_TransmitReceive(&hspi4, statusRegTx, statusRegRx, STATUS_REGISTER_BUFFER_SIZE, SPI_HAL_TIMEOUT);

	if(spiStatus != HAL_OK)
	{
		printf("Failed reading status register!\r\n");
	}

	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
	return statusRegRx[1];
}

uint32_t little_to_big_endian(uint16_t address)
{
	return (uint16_t)(((address>>8) & 0x00ff) | ((address<<8) & 0xff00));
}

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
