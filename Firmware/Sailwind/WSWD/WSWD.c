/*
 * WSWD.c
 *
 *  Created on: Aug 26, 2023
 *      Author: nicof
 */

#include <stdio.h>
#include <stdint.h>
#include "WSWD.h"
#include "UART.h"
#include "main.h"

#define GET_SERIAL_NUMBER     "02"
#define WSWD_ID               "00"
#define SIZE_OF_WSWD_ID       2U
#define SIZE_OF_WSWD_ANSWER   11U
#define SIZE_OF_WSWD_COMMAND  6U
#define SIZE_OF_WSWD_PARAM    5U

static void WSWD_enable_receive(void);
static void WSWD_enable_send(void);

static void WSWD_enable_receive(void) {
  if (HAL_GPIO_ReadPin(Windsensor_EN_GPIO_Port, Windsensor_EN_Pin)
      != GPIO_PIN_RESET) {
    HAL_GPIO_WritePin(Windsensor_EN_GPIO_Port, Windsensor_EN_Pin, GPIO_PIN_RESET);
  }
  else{
    printf("WSWD is already in receiving mode\r\n");
  }
}

static void WSWD_enable_send(void) {
  if (HAL_GPIO_ReadPin(Windsensor_EN_GPIO_Port, Windsensor_EN_Pin)
      != GPIO_PIN_SET) {
    HAL_GPIO_WritePin(Windsensor_EN_GPIO_Port, Windsensor_EN_Pin, GPIO_PIN_SET);
  }
  else{
    printf("WSWD is already in sending mode\r\n");
  }
}

uint8_t WSWD_send_without_param(char* command)
{
  assert(command != 0);
  if((sizeof(command) - 1) > SIZE_OF_WSWD_COMMAND)
  {
    return 1;
    printf("WSWD command has invalid size\r\n");
  }
  WSWD_enable_send();
  char command_structure[SIZE_OF_WSWD_COMMAND];
  snprintf(command_structure, SIZE_OF_WSWD_COMMAND, "%s%s\r\n", WSWD_ID, command);
  if(HAL_UART_Transmit(&huart2, (uint8_t*)command, SIZE_OF_WSWD_COMMAND, UART_TX_TIME_OUT) != HAL_OK)
  {
    printf("error sending to WSWD\r\n");
  }
  return HAL_OK;
}

uint8_t WSWD_send_with_param(char* command, char* param)
{
  assert(command != 0);
  if((sizeof(command) - 1) > SIZE_OF_WSWD_COMMAND || (sizeof(param) - 1) > SIZE_OF_WSWD_PARAM)
  {
    return 1;
    printf("WSWD command or param has invalid size\r\n");
  }
  WSWD_enable_send();
  char command_structure[SIZE_OF_WSWD_COMMAND];
  snprintf(command_structure, SIZE_OF_WSWD_COMMAND, "%s%s%s\r\n", WSWD_ID, command, param);
  if(HAL_UART_Transmit(&huart2, (uint8_t*)command, SIZE_OF_WSWD_COMMAND, UART_TX_TIME_OUT ) != HAL_OK)
  {
    printf("error sending to WSWD\r\n");
  }
  return HAL_OK;
}

uint8_t WSWD_receive(char* receive_buffer)
{
  WSWD_enable_receive();
  if(HAL_UART_Receive(&huart2, (uint8_t*)receive_buffer, SIZE_OF_WSWD_ANSWER, UART_TX_TIME_OUT) != HAL_OK)
  {
    printf("error sending to WSWD\r\n");
  }
  return HAL_OK;
}

uint8_t WSWD_init(void)
{
  char receive_buffer[SIZE_OF_WSWD_ANSWER];
  WSWD_enable_send();
  WSWD_send_without_param(GET_SERIAL_NUMBER);
  WSWD_receive(receive_buffer);
  printf("WSWD_ID:%s", receive_buffer);
  return HAL_OK;
}
