/**
 * \file FRAM.c
 * @author finkbeiner
 * @date 18 Jun 2023
 * @brief Interface FRAM
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
#define WEL_SET 2U
#define STATUS_REGISTER_BUFFER_SIZE 2U

/**
 * @brief check status register of FRAM
 * @param None
 * @retval FRAM status register
 */
static uint8_t FRAM_read_status_register(void);

/**
 * @brief Set WEL of FRAM
 * @param None
 * @retval FRAM status
 */
static uint8_t FRAM_write_enable(void);

uint8_t FRAM_write(uint8_t *pStructToSave, const uint16_t startAddress,
                   uint16_t sizeInByte) {
  HAL_SPI_StateTypeDef spiStatus;

  uint8_t command = WRITE;

  assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);
  assert(pStructToSave != 0);
  assert(sizeInByte != 0);

  if (FRAM_read_status_register() != WEL_SET) {
    printf("WEL not set!\r\n");
    FRAM_write_enable();
  }
  HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
  spiStatus = HAL_SPI_Transmit(&hspi4, &command, 1U, SPI_HAL_TIMEOUT);
  if (spiStatus != (HAL_SPI_StateTypeDef) HAL_OK) {
    printf("Failed sending Write instruction!\r\n");
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    return FRAM_ERROR;
  }

  spiStatus = HAL_SPI_Transmit(&hspi4, (uint8_t*) &startAddress,
                               sizeof(startAddress), SPI_HAL_TIMEOUT);
  if (spiStatus != (HAL_SPI_StateTypeDef) HAL_OK) {
    printf("Failed sending address!\r\n");
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    return FRAM_ERROR;
  }

  spiStatus = HAL_SPI_Transmit(&hspi4, pStructToSave, sizeInByte,
                               SPI_HAL_TIMEOUT);
  if (spiStatus != (HAL_SPI_StateTypeDef) HAL_OK) {
    printf("Failed sending data to be saved!\r\n");
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    return FRAM_ERROR;
  }

  HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);

  return FRAM_OK;

}

uint8_t FRAM_read(uint16_t startAddress, uint8_t *pData, uint16_t sizeInByte) {
  HAL_SPI_StateTypeDef spiStatus;
  uint8_t tx_dummy;
  uint8_t command = READ;

  assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);
  assert(pData != 0);
  assert(sizeInByte != 0);

  HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);

  spiStatus = HAL_SPI_Transmit(&hspi4, &command, 1U, SPI_HAL_TIMEOUT);

  if (spiStatus != (HAL_SPI_StateTypeDef) HAL_OK) {
    printf("Failed sending READ!\r\n");
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    return FRAM_ERROR;
  }

  spiStatus = HAL_SPI_Transmit(&hspi4, (uint8_t*) &startAddress,
                               sizeof(startAddress), SPI_HAL_TIMEOUT);
  if (spiStatus != (HAL_SPI_StateTypeDef) HAL_OK) {
    printf("Failed sending address!\r\n");
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    return FRAM_ERROR;
  }

  spiStatus = HAL_SPI_TransmitReceive(&hspi4, &tx_dummy, pData, sizeInByte,
                                      SPI_HAL_TIMEOUT);
  if (spiStatus != (HAL_SPI_StateTypeDef) HAL_OK) {
    printf("Failed receiving data!\r\n");
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    return FRAM_ERROR;
  }

  HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);

  return FRAM_OK;
}

static uint8_t FRAM_write_enable(void) {
  HAL_SPI_StateTypeDef spiStatus;
  uint8_t command = WREN;

  assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);

  HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
  spiStatus = HAL_SPI_Transmit(&hspi4, &command, 1U, SPI_HAL_TIMEOUT);

  if (spiStatus != (HAL_SPI_StateTypeDef) HAL_OK) {
    printf("Failed setting WREN!\r\n");
    HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    return FRAM_ERROR;
  }

  HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
  return FRAM_OK;
}

uint8_t FRAM_init(void) {
  printf("Starting FRAM init\r\n");

  if (FRAM_write_enable() != FRAM_OK) {
    printf("FRAM init failed!\r\n");
    return FRAM_ERROR;
  }

  if (FRAM_read_status_register() != 2U) {
    printf("Failed setting WREN!\r\n");
    return FRAM_ERROR;
  }
  printf("FRAM init completed\r\n");

  return FRAM_OK;
}

static uint8_t FRAM_read_status_register() {
  HAL_SPI_StateTypeDef spiStatus;
  uint8_t statusRegTx = RDSR;
  uint8_t statusRegRx[2] = {0};

  assert(HAL_GPIO_ReadPin(SPI4_CS_GPIO_Port, SPI4_CS_Pin) != 0);

  HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);

  spiStatus = HAL_SPI_TransmitReceive(&hspi4, &statusRegTx, statusRegRx,
                                      STATUS_REGISTER_BUFFER_SIZE,
                                      SPI_HAL_TIMEOUT);

  if (spiStatus != (HAL_SPI_StateTypeDef) HAL_OK) {
    printf("Failed reading status register!\r\n");
  }

  HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);

  return statusRegRx[1];
}
