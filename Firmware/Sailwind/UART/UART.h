/*
 * UART.h
 *
 *  Created on: 01.08.2023
 *      Author: Bene
 */

#ifndef UART_UART_H_
#define UART_UART_H_

#include "boolean.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>

#define UART_RX_TIME_OUT 1
#define UART_TX_TIME_OUT 10

boolean_t UART_receive(UART_HandleTypeDef *huart_ptr, char *Rx_buffer, uint16_t size);
void UART_transmit_ln(UART_HandleTypeDef *huart_ptr, char *Tx_buffer);
void UART_transmit_ln_int(UART_HandleTypeDef *huart_ptr, char *f_string, int32_t value);
void UART_transmit_ln_float(UART_HandleTypeDef *huart_ptr, char *f_string, float value);

#endif /* UART_UART_H_ */
