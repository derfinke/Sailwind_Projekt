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
#include "stdlib.h"

#define WSWD_ID                         "00"
#define SIZE_OF_WSWD_ID                 2U
#define SIZE_OF_WSWD_ANSWER             11U
#define SIZE_OF_NMEA_TELEGRAM           30U
#define SIZE_OF_WSWD_COMMAND            6U
#define SIZE_OF_WSWD_COMMAND_WITH_PARAM 7U
#define SIZE_OF_WSWD_PARAM              5U
#define WSWD_UART_TIMEOUT               2000U

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
  char command_structure[SIZE_OF_WSWD_COMMAND];
  snprintf(command_structure, SIZE_OF_WSWD_COMMAND, "%s%s\r\n", WSWD_ID, command);
  if(HAL_UART_Transmit(&huart2, (uint8_t*)command_structure, SIZE_OF_WSWD_COMMAND, WSWD_UART_TIMEOUT) != HAL_OK)
  {
    printf("error sending to WSWD\r\n");
  }
  WSWD_enable_receive();
  return HAL_OK;
}

uint8_t WSWD_send_with_param(char* command, char* param)
{
  assert(command != 0);
  assert(param != 0);

  WSWD_enable_send();
  char command_structure[SIZE_OF_WSWD_COMMAND_WITH_PARAM];
  snprintf(command_structure, SIZE_OF_WSWD_COMMAND_WITH_PARAM, "%s%s%s\r\n", WSWD_ID, command, param);
  if(HAL_UART_Transmit(&huart2, (uint8_t*)command_structure, SIZE_OF_WSWD_COMMAND_WITH_PARAM, WSWD_UART_TIMEOUT ) != HAL_OK)
  {
    printf("error sending to WSWD\r\n");
  }
  WSWD_enable_receive();
  return HAL_OK;
}

uint8_t WSWD_receive(char* receive_buffer, uint8_t size_of_receive_buffer)
{
  WSWD_enable_receive();
  if(HAL_UART_Receive(&huart2, (uint8_t*)receive_buffer, size_of_receive_buffer, WSWD_UART_TIMEOUT) != HAL_OK)
  {
    printf("error sending to WSWD\r\n");
  }
  return HAL_OK;
}

uint8_t WSWD_receive_NMEA(char* receive_buffer)
{
  WSWD_enable_receive();
  if(HAL_UART_Receive(&huart2, (uint8_t*)receive_buffer, SIZE_OF_NMEA_TELEGRAM, WSWD_UART_TIMEOUT) != HAL_OK)
  {
    printf("error receiving from WSWD\r\n");
  }
  return HAL_OK;
}

void WSWD_get_wind_infos(char* received_NMEA_telegramm, float Windspeed,  float Winddirection)
{
  char Windspeed_buffer[6];
  char Winddirection_buffer[5];
  if(received_NMEA_telegramm[24] == 'A')
  {
    memcpy(Winddirection_buffer, &received_NMEA_telegramm[7], 5);
    memcpy(Windspeed_buffer, &received_NMEA_telegramm[15], 6);
    Winddirection = (float)atof(Winddirection_buffer);
    Windspeed = (float)atof(Windspeed_buffer);
  }
  else
  {
    printf("error telegram invalid\r\n");
  }
}

void WSWD_get_windspeed_unit(char* received_NMEA_telegramm, char unit)
{
  unit = received_NMEA_telegramm[22];
}
