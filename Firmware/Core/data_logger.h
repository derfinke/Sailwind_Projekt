/*
 * data_logger.h
 *
 *  Created on: Jun 18, 2023
 *      Author: nicof
 */

#ifndef DATA_LOGGER_H_
#define DATA_LOGGER_H_

#include "stm32f4xx_hal_uart.h"

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

#endif /* DATA_LOGGER_H_ */
