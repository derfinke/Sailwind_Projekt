/*
 * UART.c
 *
 *  Created on: 01.08.2023
 *      Author: Bene
 */

#include "UART.h"

static int int_len(int32_t value);

boolean_t UART_receive(UART_HandleTypeDef *huart_ptr, char *Rx_buffer, uint16_t size)
{
	if (!(__HAL_UART_GET_FLAG(huart_ptr, UART_FLAG_RXNE) == SET))
	{
		return False;
	}
	HAL_UART_Receive(huart_ptr, (uint8_t *) Rx_buffer, size, UART_RX_TIME_OUT);
	return True;
}

void UART_transmit_ln(UART_HandleTypeDef *huart_ptr, char *Tx_buffer)
{
	char Tx_buffer_ln[strlen(Tx_buffer) + 2];
	sprintf(Tx_buffer_ln, "%s\r\n", Tx_buffer);
	HAL_UART_Transmit(huart_ptr, (uint8_t *)(Tx_buffer_ln), sizeof(Tx_buffer_ln), UART_TX_TIME_OUT);
}

void UART_transmit_ln_int(UART_HandleTypeDef *huart_ptr, char *f_string, int32_t value)
{
	//f_string: "__%d__"
	char Tx_buffer[strlen(f_string) + int_len(value) - 2];
	sprintf(Tx_buffer, f_string, value);
	UART_transmit_ln(huart_ptr, Tx_buffer);
}
void UART_transmit_ln_float(UART_HandleTypeDef *huart_ptr, char *f_string, float value)
{
	//f_string: "__%f__" -> cut value to two decimal parts
	//f_string: "__%.2f__" (recommended) -> round value up to two decimal parts
	char Tx_buffer[strlen(f_string) + int_len((int)value) + 1];
	sprintf(Tx_buffer, f_string, value);
	UART_transmit_ln(huart_ptr, Tx_buffer);
}

static int int_len(int32_t value)
{
  int len = !value;
  while(value)
  {
	  len++; value/=10;
  }
  return len;
}

