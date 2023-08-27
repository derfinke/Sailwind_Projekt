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

#define GET_SERIAL_NUMBER "0002\r\n"
#define WSWD_ID           "00"
#define SIZE_OF_WSWD_ANSWER 11U
#define SIZE_OF_WSWD_COMMAND 6U

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
  WSWD_enable_send();
  if(HAL_UART_Transmit(&huart2, (uint8_t*)command, SIZE_OF_WSWD_COMMAND, UART_TX_TIME_OUT) != HAL_OK)
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
